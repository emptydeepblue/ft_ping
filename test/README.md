# 🧪 ft_ping Testing Guide

This guide explains how to create and run custom test suites for the ft_ping project.

## 🛠️ Creating Custom Test Suites

You can create your own test suites to validate specific functionality or edge cases.

### Test File Format

Test files use a simple text format with test suites and commands:

```bash
# Test Suite Name
command1
command2
command3

# Another Test Suite  
command4
command5
```

### Creating a New Test Suite

1. **Create the test file** in `test/cases/`:

```bash
# Create a new test file
touch test/cases/mytest.test.txt
```

2. **Define your test cases**:

```bash
# test/cases/mytest.test.txt

# 🚀 BASIC FUNCTIONALITY
ping 127.0.0.1
ping localhost
ping -v 127.0.0.1

# 🔍 ERROR HANDLING  
ping invalid.domain.test
ping 999.999.999.999
ping -x localhost

# 🔀 DIFFERENT BEHAVIOUR
ping --help
ping -V localhost
```

### Test Suite Naming Conventions

Use descriptive emojis and names for test suites:

| Category | Example Names |
|----------|---------------|
| **Basic Tests** | ✅ VALID COMMANDS, 🚀 BASIC FUNCTIONALITY |
| **Error Cases** | ❌ INVALID FLAGS, 🚫 MISSING TARGET |
| **Edge Cases** | ⚠️ EDGE CASES, 🧪 MIXED COMBINATIONS |
| **DNS Tests** | 🌐 DNS RESOLUTION, 🧨 MALFORMED HOSTS |
| **Expected Differences** | 🔀 DIFFERENT BEHAVIOUR |

### Running Custom Tests

```bash
# Method 1: Use the Makefile pattern (recommended)
make test-mytest

# Method 2: Run the tools manually
# Generate system ping baseline
python3 test/command_collector.py test/results/mytest.results.csv test/cases/mytest.test.txt

# Generate ft_ping results  
cp ft_ping test/
cp test/cases/mytest.test.txt test/auto/ft_mytest.test.txt
sed -i 's|ping|./ft_ping|g' test/auto/ft_mytest.test.txt
python3 test/command_collector.py test/auto/ft_mytest.results.csv test/auto/ft_mytest.test.txt

# Compare results
cd test && python3 test_compare.py results/mytest.results.csv auto/ft_mytest.results.csv
```

## Advanced Test Features

### 1. **Special Test Suites**

Use `🔀 DIFFERENT BEHAVIOUR` for cases where your implementation intentionally differs:

```bash
# 🔀 DIFFERENT BEHAVIOUR
ping --help        # Your help format might differ
ping -V            # Version output might differ  
```

### 2. **Complex Commands**

Test shell features and edge cases:

```bash
# 🧪 SHELL INTEGRATION
ping "127.0.0.1"
ping '$(echo localhost)'
ping localhost && echo "Success"
ping -v 127.0.0.1 | head -1
```

### 3. **Error Validation**

Test comprehensive error handling:

```bash
# ❌ COMPREHENSIVE ERRORS
ping                    # Missing target
ping -xyz               # Invalid bundled flags
ping -- -v              # Argument after --
ping "💣"               # Unicode hostnames
```

## Test Development Workflow

1. **Identify test scenarios** you want to validate
2. **Create test file** with descriptive suite names
3. **Run system ping** to establish baseline behavior
4. **Run your implementation** and compare results
5. **Iterate** on failing tests to improve your code
6. **Document differences** in `🔀 DIFFERENT BEHAVIOUR` if intentional

## Test File Examples

Here's a complete example for IPv6 testing:

```bash
# test/cases/ipv6.test.txt

# 🌐 IPv6 BASIC TESTS
ping ::1
ping ::ffff:127.0.0.1  
ping 2001:db8::1

# ❌ IPv6 ERROR CASES
ping :::invalid
ping 2001:db8::xyz
ping [::1]:80

# 🔀 DIFFERENT BEHAVIOUR  
ping -6 ::1             # If you don't support -6 flag
```

## Debugging Test Failures

When tests fail, use these strategies:

1. **Check exact output differences**:
```bash
# Compare CSV files directly
diff test/results/mytest.results.csv test/auto/ft_mytest.results.csv
```

2. **Run individual commands**:
```bash
# Test specific command manually
ping invalid.domain.test
./ft_ping invalid.domain.test
```

3. **Examine test categories**:
```bash
# Look for patterns in failing test suites
grep "🔴" test_output.log
```

## Test Tools Overview

### command_collector.py

The test execution engine that runs commands and collects results:

- **Purpose**: Execute test commands and capture output, exit codes, and timing
- **Features**: Smart timeout handling, CSV output format, progress tracking
- **Usage**: `python3 command_collector.py output.csv input.txt`

### test_compare.py

The result comparison tool that analyzes differences:

- **Purpose**: Compare ft_ping results against system ping baseline
- **Features**: Color-coded output, statistics, special handling for expected differences
- **Usage**: `python3 test_compare.py expected.csv obtained.csv`

## Best Practices

### ✅ Do's
- **Group related tests** in logical suites
- **Use descriptive names** with emojis for visual clarity
- **Test edge cases** and error conditions thoroughly
- **Document expected differences** in `🔀 DIFFERENT BEHAVIOUR`
- **Keep tests focused** - one concept per suite when possible
- **Include both positive and negative** test cases

### ❌ Don'ts
- **Avoid overly long** individual test suites
- **Don't mix unrelated** test scenarios
- **Don't assume system ping behavior** - always generate baselines
- **Don't ignore test failures** - investigate and fix or document

## Test Suite Maintenance

### Regular Tasks

1. **Update baselines** when system ping behavior changes
2. **Review failing tests** and categorize as bugs vs expected differences
3. **Add tests** for new features or discovered edge cases
4. **Clean up** redundant or obsolete test cases

### Performance Considerations

- **Test execution time**: Aim for test suites that complete in reasonable time
- **Resource usage**: Be mindful of network tests that might impact connectivity
- **Parallel execution**: Consider test independence for potential parallelization

---

For more information about the main project, see [README.md](README.md).
