# ft_ping

A from-scratch implementation of the `ping` utility in C — raw ICMP echo requests, DNS resolution, RTT statistics, and signal-driven graceful exit.

**Specialization:** Sys Admin
**XP:** 4,200 · **People:** 1 · **Status:** 🟦 Completed

---

> **⚠️ Known behavior / limitation**
> When a target triggers an ICMP error reply (e.g. *Time to live exceeded* or
> *Destination unreachable*), `ft_ping` **without** `-v` prints only the initial
> `PING ...` header line and the final statistics — it does **not** print a line
> per error reply. The reference inetutils `ping` does print those error replies
> inline. Run `ft_ping` with `-v` to see the full ICMP error dump for each such
> reply.

---

## 🚀 Features

- Send IPv4 ICMP echo requests, parse replies, compute RTT
- DNS resolution for hostnames and dotted-quad addresses
- Statistics: packets sent / received, packet loss, min / avg / max / stddev
- Verbose mode (`-v`)
- Multiple-host argument support
- Graceful `Ctrl+C` handling: print summary and exit
- Reference-based test suite

## 🛠️ Build & Run

**Prerequisites:** GCC, GNU Make, Linux/Unix, root (raw sockets)

```bash
make                          # Build ./ft_ping

sudo ./ft_ping google.com     # Basic
sudo ./ft_ping -v localhost   # Verbose
sudo ./ft_ping 8.8.8.8 1.1.1.1  # Multiple targets
sudo ./ft_ping -?             # Help
```

**Sample output:**
```
PING google.com (142.250.180.142): 56 data bytes
64 bytes from 142.250.180.142: icmp_seq=0 ttl=114 time=20.825 ms
^C--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 20.825/20.825/20.825/0.000 ms
```

## 🧪 Testing

```bash
sudo python3 test/tester.py              # Diff-vs-reference test runner
sudo valgrind --leak-check=full ./ft_ping google.com
```

Test results: 🟢 Passed | 🔴 Failed | ⚫ Different behavior vs. reference

See [test/](test/) for case definitions and reference outputs.

## 📁 Project Structure

```
ft_ping/
├── src/
│   ├── main.c          # Entry, signal setup, main loop
│   ├── parser.c        # CLI argument parsing
│   ├── dns.c           # Hostname → IP via getaddrinfo
│   ├── ping.c          # Raw socket, ICMP send/recv, timing
│   ├── print.c         # Output formatting and final stats
│   └── utils.c         # Helpers
├── inc/ft_ping.h
├── test/
│   ├── cases/          # Test inputs
│   ├── ref/            # Reference outputs
│   └── tester.py       # Test driver
└── Makefile
```

## 🔧 Implementation

**Core components:**
- **Parser** — CLI flags and target list, input validation
- **DNS** — `getaddrinfo()` for hostname → IPv4 resolution
- **ICMP** — raw socket, echo-request crafting, RTT measurement
- **Output** — live per-reply lines, summary statistics on exit

**Technical details:**
- Raw sockets (`SOCK_RAW`, `IPPROTO_ICMP`) — requires root or `CAP_NET_RAW`
- ICMP Echo Request / Reply per [RFC 792](https://tools.ietf.org/html/rfc792)
- Timing via `gettimeofday()`; stddev computed over the RTT sample
- `SIGINT` handler triggers summary print and clean exit

## 📚 Learning Objectives

- Low-level network programming with raw sockets
- ICMP and the IPv4 header layout
- DNS resolution via `getaddrinfo`
- Signal handling and graceful shutdown
- Building a reference-diff test harness

## 🔗 References

- [RFC 792 — Internet Control Message Protocol](https://tools.ietf.org/html/rfc792)
- `man 7 raw`, `man 7 icmp`, `man 3 getaddrinfo`
- [iputils ping source](https://github.com/iputils/iputils) — reference implementation

---

*42 School — RNCP7 · Sys Admin*
