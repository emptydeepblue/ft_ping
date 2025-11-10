#!/usr/bin/env python3
"""
Test comparison tool for ft_ping parser results.
Compares parser.results.csv (expected) vs ft_parser.results.csv (obtained).
"""

import csv
import sys
import argparse
from collections import defaultdict

# ANSI color codes for terminal formatting
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    GRAY = '\033[90m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    RESET = '\033[0m'
    YELLOW = '\033[93m'

def load_test_results(filename):
    """Load test results from CSV file into a dictionary organized by test suite."""
    results = defaultdict(list)
    
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                test_suite = row['Test_Suite']
                results[test_suite].append({
                    'command': row['Command'],
                    'exit_code': int(row['Exit_Code']),
                    'output': row['Output'],
                    'execution_time': int(row['Execution_Time_ms'])
                })
    except FileNotFoundError:
        print(f"Error: File {filename} not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        sys.exit(1)
    
    return results

def tests_match(expected, obtained):
    """Check if two test results match (exit code and output)."""
    return (expected['exit_code'] == obtained['exit_code'] and 
            expected['output'].strip() == obtained['output'].strip())

def format_test_result(test, prefix, is_passed, is_different_behaviour=False):
    """Format a single test result with colors."""
    if is_passed:
        color = Colors.GREEN
    elif is_different_behaviour:
        color = Colors.GRAY
    else:
        color = Colors.RED
    
    output = f"{color}{prefix}{Colors.RESET} {test['output']}\n"
    output += f"   {Colors.GRAY}with exit code {test['exit_code']}{Colors.RESET}\n"
    output += f"   {Colors.GRAY}in [{test['execution_time']}] ms{Colors.RESET}\n"
    
    return output

def normalize_command(command):
    """Normalize command by replacing 'ping' with './ft_ping' for comparison."""
    if command.startswith('ping '):
        return 'sudo ./ft_ping ' + command[5:]
    elif command == 'ping':
        return 'sudo ./ft_ping'
    return command

def compare_test_suites(expected_results, obtained_results):
    """Compare test results and generate formatted output."""
    summary = {
        'total_tests': 0,
        'passed_tests': 0,
        'failed_tests': 0,
        'missing_tests': 0,
        'different_behaviour_tests': 0,
        'suite_breakdown': {},
        'has_different_behaviour': False
    }
    
    # Process all test suites
    all_suites = set(expected_results.keys()) | set(obtained_results.keys())
    
    # Separate the "🔀 DIFFERENT BEHAVIOUR" suite to process it last
    different_behaviour_suite = "🔀 DIFFERENT BEHAVIOUR"
    regular_suites = [suite for suite in sorted(all_suites) if suite != different_behaviour_suite]
    
    # Process regular suites first
    for suite in regular_suites:
        _process_test_suite(suite, expected_results, obtained_results, summary)
    
    # Process "🔀 DIFFERENT BEHAVIOUR" suite last if it exists
    if different_behaviour_suite in all_suites:
        summary['has_different_behaviour'] = True
        _process_test_suite(different_behaviour_suite, expected_results, obtained_results, summary)
    
    return summary

def _process_test_suite(suite, expected_results, obtained_results, summary):
    """Process a single test suite."""
    print(f"\n{Colors.BOLD}{Colors.UNDERLINE}{suite}{Colors.RESET}\n")
    
    # Check if this is the different behaviour suite
    is_different_behaviour = suite == "🔀 DIFFERENT BEHAVIOUR"
    
    suite_stats = {'total': 0, 'passed': 0, 'failed': 0}
    
    expected_tests = expected_results.get(suite, [])
    obtained_tests = obtained_results.get(suite, [])
    
    # Create a mapping of normalized commands to test results for easier comparison
    expected_map = {}
    for test in expected_tests:
        normalized_cmd = normalize_command(test['command'])
        expected_map[normalized_cmd] = test
    
    obtained_map = {test['command']: test for test in obtained_tests}
    
    # Get all unique commands from both sets
    all_commands = set(expected_map.keys()) | set(obtained_map.keys())
    
    for command in sorted(all_commands):
        summary['total_tests'] += 1
        suite_stats['total'] += 1
        
        expected = expected_map.get(command)
        obtained = obtained_map.get(command)
        
        # Display the original command from expected results
        original_cmd = None
        if expected:
            # Find the original command that matches this normalized command
            for test in expected_tests:
                if normalize_command(test['command']) == command:
                    original_cmd = test['command']
                    break
        else:
            original_cmd = command
            
        if expected is None:
            mark_color = Colors.GRAY if is_different_behaviour else Colors.RED
            print(f"{mark_color}✗{Colors.RESET} - {Colors.BOLD}Command:{Colors.RESET} {original_cmd}")
            print(f"{Colors.YELLOW}Warning: Test only exists in obtained results{Colors.RESET}")
            print(format_test_result(obtained, "[-]", False, is_different_behaviour))
            
            if is_different_behaviour:
                summary['different_behaviour_tests'] += 1
            else:
                summary['failed_tests'] += 1
            suite_stats['failed'] += 1
        elif obtained is None:
            mark_color = Colors.GRAY if is_different_behaviour else Colors.RED
            print(f"{mark_color}✗{Colors.RESET} - {Colors.BOLD}Command:{Colors.RESET} {original_cmd}")
            print(f"{Colors.YELLOW}Warning: Test missing in obtained results{Colors.RESET}")
            print(format_test_result(expected, "[+]", False, is_different_behaviour))
            summary['missing_tests'] += 1
            
            if is_different_behaviour:
                summary['different_behaviour_tests'] += 1
            else:
                summary['failed_tests'] += 1
            suite_stats['failed'] += 1
        else:
            # Both tests exist, compare them
            is_passed = tests_match(expected, obtained)
            
            if is_passed:
                summary['passed_tests'] += 1
                suite_stats['passed'] += 1
                mark = f"{Colors.GREEN}✓{Colors.RESET}"
            else:
                suite_stats['failed'] += 1
                mark_color = Colors.GRAY if is_different_behaviour else Colors.RED
                mark = f"{mark_color}✗{Colors.RESET}"
                
                if is_different_behaviour:
                    summary['different_behaviour_tests'] += 1
                else:
                    summary['failed_tests'] += 1
            
            print(f"{mark} - {Colors.BOLD}Command:{Colors.RESET} {original_cmd}")
            print(format_test_result(expected, "[+]", is_passed, is_different_behaviour))
            print(format_test_result(obtained, "[-]", is_passed, is_different_behaviour))
        
        print()  # Empty line between tests
    
    summary['suite_breakdown'][suite] = suite_stats

def print_summary(summary):
    """Print the test results summary."""
    print(f"\n{Colors.BOLD}{Colors.UNDERLINE}SUMMARY{Colors.RESET}")
    print(f"Total tests: {summary['total_tests']}")
    print(f"{Colors.GREEN}Passed: {summary['passed_tests']}{Colors.RESET}")
    print(f"{Colors.RED}Failed: {summary['failed_tests']}{Colors.RESET}")
    
    if summary['different_behaviour_tests'] > 0:
        print(f"{Colors.GRAY}Different Behaviour: {summary['different_behaviour_tests']}{Colors.RESET}")
    
    if summary['missing_tests'] > 0:
        print(f"{Colors.YELLOW}Missing: {summary['missing_tests']}{Colors.RESET}")
    
    if summary['total_tests'] > 0:
        # Calculate pass rate excluding different behaviour tests
        actual_testable = summary['total_tests'] - summary['different_behaviour_tests']
        if actual_testable > 0:
            pass_rate = (summary['passed_tests'] / actual_testable) * 100
        else:
            pass_rate = 100.0
        
        color = Colors.GREEN if pass_rate >= 80 else Colors.YELLOW if pass_rate >= 50 else Colors.RED
        print(f"Pass rate: {color}{pass_rate:.1f}%{Colors.RESET}")
        
        # Additional breakdown
        print(f"\n{Colors.BOLD}Test Categories Breakdown:{Colors.RESET}")
        if summary.get('suite_breakdown'):
            for suite, stats in summary['suite_breakdown'].items():
                suite_pass_rate = (stats['passed'] / stats['total']) * 100 if stats['total'] > 0 else 0
                
                # Use grey color for different behaviour section
                if suite == "🔀 DIFFERENT BEHAVIOUR":
                    suite_color = Colors.GRAY
                else:
                    suite_color = Colors.GREEN if suite_pass_rate >= 80 else Colors.YELLOW if suite_pass_rate >= 50 else Colors.RED
                
                print(f"  {suite}: {suite_color}{stats['passed']}/{stats['total']} ({suite_pass_rate:.0f}%){Colors.RESET}")
    
    # Special message for different behaviour section
    if summary.get('has_different_behaviour'):
        print(f"\n{Colors.YELLOW}{Colors.BOLD}📋 NOTICE:{Colors.RESET}")
        print(f"{Colors.YELLOW}Differences in '🔀 DIFFERENT BEHAVIOUR' section are documented and expected.{Colors.RESET}")
        print(f"{Colors.YELLOW}These represent intentional variations from the system ping behavior.{Colors.RESET}")

def main():
    """Main function to run the comparison."""
    parser = argparse.ArgumentParser(description='Compare ft_ping test results')
    parser.add_argument('expected_file', help='Path to expected results CSV file')
    parser.add_argument('obtained_file', help='Path to obtained results CSV file')
    
    args = parser.parse_args()
    
    print(f"{Colors.BOLD}ft_ping Test Results Comparison{Colors.RESET}")
    print(f"{Colors.BOLD}Expected:{Colors.RESET} {args.expected_file}")
    print(f"{Colors.BOLD}Obtained:{Colors.RESET} {args.obtained_file}")
    print("=" * 60)
    
    # Load test results
    expected_results = load_test_results(args.expected_file)
    obtained_results = load_test_results(args.obtained_file)
    
    # Compare and display results
    summary = compare_test_suites(expected_results, obtained_results)
    
    # Print summary
    print_summary(summary)

if __name__ == "__main__":
    main()
