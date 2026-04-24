# ft_ping

A custom implementation of the `ping` utility in C, implementing ICMP protocols, DNS resolution, and network programming concepts.

## 🚀 Features

- Send ICMP echo requests to hosts (IPv4)
- DNS resolution for hostnames and IP addresses  
- Statistics reporting (packet loss, min/avg/max/stddev timing)
- Verbose mode with `-v` flag
- Multiple host support
- Signal handling with `Ctrl+C`
- Comprehensive test suite

## 🛠️ Build & Run

**Prerequisites**: GCC, Make, Linux/Unix, Root privileges

```bash
# Build
make

# Basic usage
sudo ./ft_ping google.com

# Verbose mode  
sudo ./ft_ping -v localhost

# Multiple hosts
sudo ./ft_ping google.com 8.8.8.8

# Help
sudo ./ft_ping -?
```

**Sample Output:**
```
PING google.com (142.250.180.142): 56 data bytes
64 bytes from 142.250.180.142: icmp_seq=0 ttl=114 time=20.825 ms
^C--- google.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 20.825/20.825/20.825/0.000 ms
```

## 🧪 Testing

```bash
# Run test suites
make test-parser    # Argument parsing
make test-dns       # DNS resolution

# Valgrind memory checking
sudo valgrind --leak-check=full ./ft_ping google.com
```

Test results: 🟢 Passed | 🔴 Failed | ⚫ Different Behavior

See [test/README.md](test/README.md) for creating custom test suites.

## 📁 Project Structure

```
ft_ping/
├── src/           # Source code (main.c, parser.c, dns.c, ping.c, print.c, utils.c)
├── inc/           # Headers (ft_ping.h)
├── test/          # Test suite and documentation  
├── Makefile       # Build configuration
└── README.md      # This file
```

## 🔧 Implementation

**Core Components:**
- **Parser**: Command-line argument validation and option handling
- **DNS**: Hostname to IP resolution using `getaddrinfo()`  
- **ICMP**: Raw socket creation, packet crafting, and RTT measurement
- **Output**: Statistics calculation and formatted display

**Technical Details:**
- Uses raw sockets (requires root privileges)
- Implements RFC 792 ICMP Echo Request/Reply
- Signal handling for graceful interruption
- Precise timing with `gettimeofday()`

##  Learning Objectives

- Low-level network programming with raw sockets
- ICMP protocol implementation (RFC 792)
- DNS resolution and IPv4 address handling
- Signal handling and process management
- Automated testing and validation
- Memory management and debugging
