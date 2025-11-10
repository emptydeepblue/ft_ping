# ft_ping

A custom implementation of the `ping` utility, built as a networking project to understand ICMP protocols, DNS resolution, and network programming in C.

## 📋 Project Overview

**ft_ping** is a recreation of the standard Unix `ping` command that sends ICMP Echo Request packets to network hosts and measures their response times. This project implements core networking concepts including:

- **ICMP Protocol**: Sending and receiving Internet Control Message Protocol packets
- **DNS Resolution**: Converting hostnames to IP addresses using system DNS
- **Raw Sockets**: Low-level network programming with packet crafting
- **Network Timing**: Precise measurement of round-trip times (RTT)
- **Signal Handling**: Graceful interruption and statistics reporting
- **Error Handling**: ICMP error message detection and reporting

## 🚀 Features

- ✅ **Basic ping functionality** - Send ICMP echo requests to hosts
- ✅ **DNS resolution** - Support for both hostnames and IP addresses  
- ✅ **Statistics reporting** - Packet loss, min/avg/max/stddev timing
- ✅ **Verbose mode** - Detailed packet information with `-v` flag
- ✅ **Signal handling** - Clean exit with `Ctrl+C` showing statistics
- ✅ **IPv4 support** - Full IPv4 address family support
- ✅ **Multiple targets** - Support for pinging multiple hosts in sequence
- ✅ **ICMP error reporting** - Detection of destination unreachable and time exceeded
- ✅ **Comprehensive testing** - Automated test suite for validation
- ✅ **Memory leak detection** - Valgrind integration for memory management

## 🛠️ Building the Project

### Prerequisites

- **GCC** compiler with C99 support
- **Make** build system
- **Linux/Unix** environment (requires raw socket capabilities)
- **Root privileges** (required for ICMP packet creation)

### Compilation

```bash
# Clone or navigate to project directory
git clone https://github.com/minestrinad/ft_ping.git
cd ft_ping

# Build the project
make

# Clean build artifacts
make clean

# Complete rebuild
make re
```

The build process creates the `ft_ping` executable in the project root.

## 📖 Usage

### Basic Syntax

```bash
sudo ./ft_ping [OPTIONS] HOST
```

**Note**: Root privileges are required for raw socket operations.

### Examples

```bash
# Basic ping to a hostname
sudo ./ft_ping google.com

# Ping an IP address
sudo ./ft_ping 8.8.8.8

# Verbose mode with detailed packet info
sudo ./ft_ping -v localhost

# Multiple verbose flags for extra verbosity
sudo ./ft_ping -vvv google.com

# Ping multiple hosts in sequence
sudo ./ft_ping google.com yahoo.com 8.8.8.8

# Help and usage information
sudo ./ft_ping -?
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-v` | Verbose mode - show detailed packet information including packet ID |
| `-vvv` | Multiple verbose flags for enhanced verbosity |
| `-?` | Display help/usage information |
| `HOST` | Target hostname or IP address (supports multiple hosts) |

### Sample Output

```
PING google.com (142.250.180.142): 56 data bytes
64 bytes from 142.250.180.142: icmp_seq=0 ttl=114 time=20.825 ms
64 bytes from 142.250.180.142: icmp_seq=1 ttl=114 time=22.256 ms
^C--- google.com ping statistics ---
2 packets transmitted, 2 packets received, 0% packet loss
round-trip min/avg/max/stddev = 20.825/21.541/22.256/0.716 ms
```

### Verbose Mode Output

In verbose mode (`-v`), additional information is displayed:

```
PING localhost (127.0.0.1): 56 data bytes, id 0x1234 = 4660
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.123 ms
```

## 🧪 Testing

The project includes a comprehensive test suite that validates functionality against the system `ping` command.

### Running Tests

```bash
# Test argument parsing and error handling
make test-parser

# Test DNS resolution functionality  
make test-dns  

# Test ping functionality and network connectivity
make test-ping

# Test failure cases and edge conditions
make test-fail

# Run valgrind memory checking on any test
make vtest-ping
make vtest-parser

# Run valgrind directly on the executable
make valgrind
```

### Test Categories

- **Parser Tests**: Command-line argument validation, option parsing, and error handling
- **DNS Tests**: Hostname resolution, invalid domain handling, and IPv4 address validation  
- **Ping Tests**: Network connectivity, ICMP functionality, and round-trip timing
- **Fail Tests**: Edge cases, malformed inputs, and error condition handling

### Test Output

Tests compare `ft_ping` behavior against system `ping` and categorize results:

- 🟢 **Passed**: Behavior matches system ping exactly
- 🔴 **Failed**: Behavior differs from system ping significantly
- ⚫ **Different Behavior**: Documented intentional differences (marked with `🔀`)

### Valgrind Testing

Memory leak detection and analysis:

```bash
# Run valgrind on specific test suite
make vtest-ping

# Direct valgrind execution
sudo valgrind --leak-check=full ./ft_ping google.com
```

### Custom Test Suites

For detailed information on creating your own test suites, see [test/README.md](test/README.md).

## 📁 Project Structure

```
ft_ping/
├── src/                    # Source code
│   ├── main.c             # Main program entry and global variables
│   ├── parser.c           # Argument parsing and validation
│   ├── dns.c              # DNS resolution and host validation
│   ├── ping.c             # ICMP ping logic and socket handling
│   ├── print.c            # Output formatting and error reporting
│   └── utils.c            # Utility functions and statistics
├── inc/                    # Header files
│   └── ft_ping.h          # Main header with structs and prototypes
├── obj/                    # Build artifacts (created during build)
├── test/                   # Comprehensive test suite
│   ├── cases/             # Test case definitions (.test.txt files)
│   ├── results/           # System ping baseline results (.results.csv)
│   ├── tests/             # Test execution and comparison tools
│   │   ├── command_collector.py # Test execution engine
│   │   ├── test_compare.py     # Result comparison tool
│   │   └── run_and_parse_valgrind.py # Valgrind test runner
│   └── README.md          # Testing guide and documentation
├── Makefile               # Build configuration with test targets
└── README.md              # This file
```

## 🔧 Implementation Details

### Core Components

1. **Argument Parser** (`parser.c`)
   - Validates command-line arguments with comprehensive error checking
   - Handles help/usage flags (`-?`) and verbose modes (`-v`, `-vvv`)
   - Extracts target hosts and options with support for multiple targets
   - Implements proper flag bundling and validation

2. **DNS Resolver** (`dns.c`)  
   - Converts hostnames to IPv4 addresses using `getaddrinfo()`
   - Handles IPv4 address validation and format checking
   - Provides detailed error reporting for resolution failures
   - Supports both numeric IPs and domain names

3. **ICMP Engine** (`ping.c`)
   - Creates raw ICMP sockets with proper error handling
   - Crafts Echo Request packets with unique identifiers
   - Implements precise round-trip time measurement
   - Handles packet reception, validation, and ownership checking
   - Supports timeout handling and retry logic

4. **Output System** (`print.c`)
   - Formats ping output to match system ping behavior
   - Handles verbose mode with enhanced packet information
   - Provides comprehensive error reporting for ICMP errors
   - Generates statistics summary with mathematical precision

5. **Statistics & Utilities** (`utils.c`)
   - Tracks sent/received packet counts with loss calculation
   - Calculates timing statistics (min/avg/max/standard deviation)
   - Handles graceful signal interruption (SIGINT)
   - Implements memory management and cleanup functions

### Technical Considerations

- **Raw Sockets**: Requires `CAP_NET_RAW` capability or root privileges for ICMP operations
- **ICMP Protocol**: Implements RFC 792 Echo Request/Reply with proper packet validation
- **DNS Resolution**: Uses `getaddrinfo()` for hostname lookup with IPv4 family filtering
- **Signal Handling**: Responds to `SIGINT` (Ctrl+C) with comprehensive statistics display
- **Memory Management**: Careful handling of network buffers, linked lists, and dynamic structures
- **Packet Validation**: Implements checksum calculation and packet ownership verification
- **Error Detection**: Handles ICMP error messages (Destination Unreachable, Time Exceeded)
- **Timing Precision**: Uses `gettimeofday()` for microsecond-level RTT measurement

## 🐛 Known Limitations

- **IPv6 Support**: Limited to IPv4 addresses (IPv6 planned for future versions)
- **Advanced Options**: Some standard `ping` options not implemented
- **Platform Support**: Designed and tested for Linux/Unix environments
- **Packet Size**: Fixed 64-byte packet size (including ICMP header)
- **Single Target**: Pings hosts sequentially rather than simultaneously

## 📚 Learning Objectives

This project demonstrates understanding of:

- **Low-level network programming** in C with raw sockets
- **ICMP protocol implementation** following RFC 792 specifications
- **DNS resolution mechanisms** using system APIs
- **Raw socket programming** with proper privilege handling
- **Signal handling and process management** for graceful interruption
- **Comprehensive software testing** with automated validation
- **Build system configuration** using Make with pattern-based targets
- **Memory management** and leak detection with Valgrind
- **Mathematical calculations** for network statistics (variance, standard deviation)
- **Error handling** for network failures and malformed inputs
