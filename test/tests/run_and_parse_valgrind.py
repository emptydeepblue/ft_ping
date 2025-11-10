#!/usr/bin/env python3

import subprocess
import re
import os
import concurrent.futures
import sys

# ANSI color codes for terminal formatting
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BOLD = '\033[1m'
    RESET = '\033[0m'

def run_single_test(command):
    """Run a single valgrind command and return output and status."""
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=10)
        output = result.stdout + result.stderr
        return output, False  # False for no timeout
    except subprocess.TimeoutExpired:
        return "TIMEOUT: Command took too long", True  # True for timeout

def parse_output_for_leaks(output, timed_out):
    """Parse the output for leaks and return status."""
    if timed_out:
        return True, 0  # has_dirt = True, still = 0

    leak_summary_match = re.search(r'LEAK SUMMARY:\s*\n(.*?)\n==\d+== ERROR SUMMARY', output, re.DOTALL)
    has_dirt = False
    current_still_reachable = 0
    if leak_summary_match:
        summary = leak_summary_match.group(1)
        lines_sum = summary.split('\n')
        for l in lines_sum:
            if 'definitely lost:' in l:
                match = re.search(r'definitely lost: (\d+) bytes', l)
                if match and int(match.group(1)) > 0:
                    has_dirt = True
            elif 'indirectly lost:' in l:
                match = re.search(r'indirectly lost: (\d+) bytes', l)
                if match and int(match.group(1)) > 0:
                    has_dirt = True
            elif 'possibly lost:' in l:
                match = re.search(r'possibly lost: (\d+) bytes', l)
                if match and int(match.group(1)) > 0:
                    has_dirt = True
            elif 'still reachable:' in l:
                match = re.search(r'still reachable: (\d+) bytes', l)
                if match:
                    current_still_reachable = int(match.group(1))
    return has_dirt, current_still_reachable

def run_and_check_tests(test_file, executable):
    with open(test_file, 'r') as f:
        lines = f.readlines()

    tasks = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            continue

        # Normalize: remove 'ping ' prefix if present
        if line.startswith('ping '):
            args = line[5:]
        else:
            args = line
        command = f"valgrind --leak-check=full --show-leak-kinds=all {executable} {args}"
        tasks.append((line, command))

    total_tests = len(tasks)
    passed = 0
    failed = 0
    total_definitely_lost = 0
    total_indirectly_lost = 0
    total_possibly_lost = 0
    total_still_reachable = 0

    # Run tests in parallel
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
        future_to_task = {executor.submit(run_single_test, cmd): (orig_line, cmd) for orig_line, cmd in tasks}
        for future in concurrent.futures.as_completed(future_to_task):
            orig_line, cmd = future_to_task[future]
            try:
                output, timed_out = future.result()
                has_dirt, current_still_reachable = parse_output_for_leaks(output, timed_out)
                total_still_reachable += current_still_reachable

                print(f"{Colors.YELLOW}Test: {orig_line}{Colors.RESET}")
                if has_dirt:
                    print(f"{Colors.RED}✗ Failed{Colors.RESET}")
                    failed += 1
                else:
                    print(f"{Colors.GREEN}✓ Passed{Colors.RESET}")
                    passed += 1
                    if current_still_reachable > 0:
                        print(f"{Colors.YELLOW}⚠ Warning: {current_still_reachable} bytes still reachable{Colors.RESET}")
                print("-" * 50)
            except Exception as exc:
                print(f"{Colors.YELLOW}Test: {orig_line}{Colors.RESET}")
                print(f"{Colors.RED}✗ Failed (error: {exc}){Colors.RESET}")
                failed += 1
                print("-" * 50)

    # Summary
    percent_passed = (passed / total_tests * 100) if total_tests > 0 else 0
    print(f"{Colors.BOLD}Summary:{Colors.RESET}")
    print(f"Passed: {Colors.GREEN}{passed}{Colors.RESET}")
    print(f"Failed: {Colors.RED}{failed}{Colors.RESET}")
    print(f"Pass percentage: {Colors.GREEN if percent_passed == 100 else Colors.YELLOW}{percent_passed:.2f}%{Colors.RESET}")
    def_lost_color = Colors.RED if total_definitely_lost > 0 else Colors.GREEN
    ind_lost_color = Colors.RED if total_indirectly_lost > 0 else Colors.GREEN
    pos_lost_color = Colors.RED if total_possibly_lost > 0 else Colors.GREEN
    still_color = Colors.YELLOW if total_still_reachable > 0 else Colors.GREEN
    print(f"Definitely lost: {def_lost_color}{total_definitely_lost} bytes{Colors.RESET}")
    print(f"Indirectly lost: {ind_lost_color}{total_indirectly_lost} bytes{Colors.RESET}")
    print(f"Possibly lost: {pos_lost_color}{total_possibly_lost} bytes{Colors.RESET}")
    print(f"Still reachable: {still_color}{total_still_reachable} bytes{Colors.RESET}")

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if len(sys.argv) != 2:
        print("Usage: python3 run_and_parse_valgrind.py <test_file>")
        sys.exit(1)
    test_file = sys.argv[1]
    executable = os.path.join(script_dir, "../../ft_ping")
    run_and_check_tests(test_file, executable)