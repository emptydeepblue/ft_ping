#!/usr/bin/env python3
"""
ft_ping tester — compares ft_ping output against system ping.

Usage:
    python3 test/tester.py <suite>             # parser | dns | ping
    python3 test/tester.py <suite> --valgrind  # memory leak check
    python3 test/tester.py <suite> --regen     # regenerate reference cache
"""

import json
import os
import re
import signal
import subprocess
import sys
from pathlib import Path

ROOT    = Path(__file__).resolve().parent.parent
FT_PING = ROOT / "ft_ping"
CASES   = ROOT / "test" / "cases"
REF_DIR = ROOT / "test" / "ref"

TIMEOUT = 3   # seconds before SIGINT for long-running commands (actual pinging)

GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
GRAY   = "\033[90m"
BOLD   = "\033[1m"
RESET  = "\033[0m"


def run(cmd: str) -> tuple[str, int]:
    """Run a shell command. If it hangs past TIMEOUT, send SIGINT to the whole process
    group (covers pipelines and compound commands like 'ping x && echo y'), then collect
    whatever output was produced before killing."""
    proc = subprocess.Popen(
        ["bash", "-c", cmd],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        start_new_session=True,   # isolate into its own process group
    )
    try:
        out, _ = proc.communicate(timeout=TIMEOUT)
        return out or "", proc.returncode or 0
    except subprocess.TimeoutExpired:
        try:
            os.killpg(proc.pid, signal.SIGINT)   # interrupt the whole group
        except ProcessLookupError:
            pass
        try:
            out, _ = proc.communicate(timeout=2)
            return out or "", proc.returncode or 0
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid, signal.SIGKILL)
            proc.communicate()
            return "", -1


def normalize(text: str) -> str:
    """Strip values that vary between runs so comparisons are stable."""
    t = re.sub(r"\bft_ping:", "ping:", text)                              # binary name
    t = re.sub(r"\(\d+\.\d+\.\d+\.\d+\)", "(X.X.X.X)", t)              # resolved IP in header
    t = re.sub(r", id 0x[0-9a-f]+ = \d+", ", id 0xXXXX = N", t)        # verbose PID
    t = re.sub(r"\btime=[\d.]+ ms", "time=X ms", t)                     # RTT per packet
    t = re.sub(r"\bttl=\d+\b", "ttl=X", t)                             # TTL hops
    t = re.sub(r"\bicmp_seq=\d+\b", "icmp_seq=N", t)                    # sequence numbers
    t = re.sub(r"\d+ bytes from", "N bytes from", t)                    # per-packet byte count
    t = re.sub(r"\d+ packets transmitted", "N transmitted", t)
    t = re.sub(r"\d+ (?:packets )?received", "N received", t)
    t = re.sub(r"[\d.]+% packet loss", "X% loss", t)
    t = re.sub(r"[\d.]+/[\d.]+/[\d.]+/[\d.]+ ms", "X/X/X/X ms", t)    # RTT stats
    t = re.sub(r", time \d+ms", "", t)                                  # Linux-only "time 2002ms"
    t = re.sub(r"\(\d+\) bytes of data", "N bytes of data", t)          # "(84) bytes of data"
    return t.strip()


def key_output(raw: str) -> str:
    """Distill output to the lines that matter for comparison."""
    lines = [l.strip() for l in raw.splitlines() if l.strip()]
    if not lines:
        return ""

    # For help/usage output: just keep the first "Usage:" line — our help is intentionally
    # shorter than system ping, but the first line is identical.
    if any("Usage:" in l for l in lines):
        usage_line = next(l for l in lines if "Usage:" in l)
        return normalize(usage_line)

    # For ping output: keep header + first packet line only + stats lines.
    # Dropping extra packet lines prevents flaky failures due to packet count differences.
    result = []
    seen_packet = False
    for line in lines:
        is_packet = bool(re.search(r"icmp_seq=\d+", line))
        if is_packet:
            if not seen_packet:
                result.append(line)
                seen_packet = True
        else:
            result.append(line)

    return normalize("\n".join(result))


def ft_cmd(ref_cmd: str) -> str:
    """Convert 'ping ARGS' → '/abs/path/ft_ping ARGS' (run tester as root for ping tests)."""
    if ref_cmd == "ping":
        return str(FT_PING)
    if ref_cmd.startswith("ping "):
        return f"{FT_PING} {ref_cmd[5:]}"
    return ref_cmd


def load_cases(suite: str) -> list[tuple[str, str]]:
    """Return [(section, command)] from the suite's test file."""
    path = CASES / f"{suite}.test.txt"
    if not path.exists():
        sys.exit(f"{RED}No test file found: {path}{RESET}")
    cases, section = [], "General"
    for line in path.read_text().splitlines():
        line = line.strip()
        if line.startswith("#"):
            section = line.lstrip("#").strip()
        elif line:
            cases.append((section, line))
    return cases


def generate_reference(cases: list[tuple[str, str]]) -> list[dict]:
    """Run system ping for every case and return results."""
    results = []
    n = len(cases)
    for i, (section, cmd) in enumerate(cases, 1):
        print(f"\r  [{i}/{n}] {cmd[:65]:<65}", end="", flush=True)
        out, code = run(cmd)
        results.append({"section": section, "command": cmd,
                        "exit_code": code, "output": key_output(out)})
    print()
    return results


def load_reference(suite: str, cases: list, regen: bool) -> list[dict]:
    """Return cached reference results, regenerating when missing, stale, or --regen."""
    REF_DIR.mkdir(exist_ok=True)
    ref_file = REF_DIR / f"{suite}.json"
    if not regen and ref_file.exists():
        data = json.loads(ref_file.read_text())
        if len(data) == len(cases):
            return data
    print(f"{BOLD}Generating '{suite}' reference (system ping)...{RESET}")
    results = generate_reference(cases)
    ref_file.write_text(json.dumps(results, indent=2))
    print(f"  Saved → {ref_file.relative_to(ROOT)}\n")
    return results


def run_suite(suite: str, regen: bool) -> bool:
    """Compare ft_ping against reference for every test case. Returns True if all pass."""
    cases     = load_cases(suite)
    reference = load_reference(suite, cases, regen)

    print(f"{BOLD}Suite: {suite}{RESET}\n")

    passed = failed = skipped = 0
    failures: list[tuple] = []

    for ref in reference:
        section = ref["section"]
        cmd     = ref["command"]
        is_diff = "DIFFERENT" in section.upper()

        out, code = run(ft_cmd(cmd))
        got = key_output(out)

        exit_ok   = ref["exit_code"] == code
        output_ok = ref["output"] == got
        ok = exit_ok and output_ok

        if ok:
            print(f"  {GREEN}✓{RESET} {cmd}")
            passed += 1
        elif is_diff:
            print(f"  {GRAY}~{RESET} {cmd}")
            skipped += 1
        else:
            print(f"  {RED}✗{RESET} {cmd}")
            failed += 1
            failures.append((cmd, ref, code, got, exit_ok, output_ok))

    if failures:
        print(f"\n{BOLD}─── Failures ───{RESET}")
        for cmd, ref, got_code, got_out, exit_ok, output_ok in failures:
            print(f"\n  {RED}{cmd}{RESET}")
            if not exit_ok:
                print(f"    exit  expected={ref['exit_code']}  got={got_code}")
            if not output_ok:
                for line in ref["output"].splitlines():
                    print(f"    {GREEN}+ {line}{RESET}")
                for line in got_out.splitlines():
                    print(f"    {RED}- {line}{RESET}")

    testable = passed + failed
    rate     = passed / testable * 100 if testable else 100.0
    color    = GREEN if rate == 100 else YELLOW if rate >= 80 else RED

    print(f"\n  {BOLD}{suite}:{RESET} {color}{passed}/{testable}{RESET} ({rate:.0f}%)", end="")
    if skipped:
        print(f"  {GRAY}+{skipped} different-behaviour (excluded){RESET}", end="")
    print()
    return failed == 0


def run_valgrind(suite: str) -> None:
    """Run every test case through valgrind and report memory leaks."""
    cases = load_cases(suite)
    print(f"{BOLD}Valgrind: {suite}{RESET}\n")

    passed = failed = 0
    for section, cmd in cases:
        args = cmd[5:] if cmd.startswith("ping ") else ("" if cmd == "ping" else cmd)
        vcmd = (f"valgrind --leak-check=full --show-leak-kinds=all "
                f"--error-exitcode=1 {FT_PING} {args}")
        out, _ = run(vcmd)

        m = re.search(
            r"definitely lost: ([\d,]+).*?indirectly lost: ([\d,]+).*?possibly lost: ([\d,]+)",
            out, re.DOTALL,
        )
        leaked = m and any(int(m.group(i).replace(",", "")) > 0 for i in (1, 2, 3))

        if leaked:
            print(f"  {RED}✗{RESET} {cmd}")
            print(f"    def={m.group(1)}  ind={m.group(2)}  pos={m.group(3)} bytes")
            failed += 1
        else:
            print(f"  {GREEN}✓{RESET} {cmd}")
            passed += 1

    total = passed + failed
    color = GREEN if failed == 0 else RED
    print(f"\n  {color}{passed}/{total}{RESET} clean")


def check_root(suite: str) -> None:
    """Warn if not root and the suite needs raw socket access."""
    import os
    if suite == "ping" and os.geteuid() != 0:
        print(f"{YELLOW}Warning: 'ping' suite needs raw sockets. Run as root: sudo make test-ping{RESET}\n")


def main() -> None:
    args = sys.argv[1:]
    if not args or args[0] in ("-h", "--help"):
        print(__doc__)
        return

    suite    = args[0]
    valgrind = "--valgrind" in args
    regen    = "--regen" in args

    if not (CASES / f"{suite}.test.txt").exists():
        sys.exit(f"{RED}Unknown suite '{suite}'. Available: parser | dns | ping{RESET}")

    check_root(suite)

    if valgrind:
        run_valgrind(suite)
    else:
        ok = run_suite(suite, regen)
        sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
