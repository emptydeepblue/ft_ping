#!/usr/bin/env python3
"""
Smart command execution script that collects results in CSV format.
Intelligently handles ping commands by detecting DNS failures and working pings.

Usage: ./command_collector.py [output_file.csv] [commands_file.txt]
"""

# ping -v\
#  google.com

import csv
import sys
import subprocess
import time
import signal
import re
from pathlib import Path

class CommandCollector:
    def __init__(self, output_file="test/ft_parser.results.csv", commands_file="test/parser.test.txt"):
        self.output_file = output_file
        self.commands_file = commands_file
        self.results = []
        
    def is_ping_command(self, cmd):
        """Check if command is a ping command."""
        return cmd.strip().startswith(('ping', './ft_ping'))
    
    def detect_output_type(self, output_lines):
        """Detect the type of ping output."""
        if not output_lines:
            return "empty"
        
        # Join all lines to check for patterns
        full_output = '\n'.join(output_lines)
        
        # DNS resolution failure patterns
        dns_failure_patterns = [
            r"ping: cannot resolve",
            r"ping: Name or service not known",
            r"ping: Temporary failure in name resolution", 
            r"ping: nodename nor servname provided",
            r"ping: Invalid argument",
            r"ping: Address family for hostname not supported",
            r"ft_ping: cannot resolve",
            r"ft_ping: Name or service not known",
            r"ft_ping: getaddrinfo",
            r"unknown host"
        ]
        
        for pattern in dns_failure_patterns:
            if re.search(pattern, full_output, re.IGNORECASE):
                return "dns_failure"
        
        # Check for successful ping patterns
        if re.search(r'\d+ bytes from .+: icmp_seq=\d+', full_output):
            return "working_ping"
        
        # Check for other error patterns (bad options, etc.)
        if re.search(r'(invalid|illegal|bad|unknown).*option', full_output, re.IGNORECASE):
            return "option_error"
            
        if "Usage:" in full_output or "usage:" in full_output:
            return "usage"
            
        return "other"
    
    def format_output_for_csv(self, output_lines, output_type, exit_code):
        """Format output appropriately based on type."""
        if not output_lines:
            return ""
        
        # Filter out empty lines for better formatting
        non_empty_lines = [line for line in output_lines if line.strip()]
        
        if output_type == "working_ping":
            # For working pings, take first line and statistics line
            first_line = non_empty_lines[0] if non_empty_lines else ""
            
            # Look for statistics line
            stats_line = ""
            for line in non_empty_lines:
                if re.search(r'\d+ packets transmitted', line):
                    stats_line = line
                    break
            
            if stats_line:
                result = f"{first_line} | {stats_line}"
            else:
                # Fallback to first 2 lines
                result = ' | '.join(non_empty_lines[:2])
                
        else:
            # For all other cases (errors, usage, etc.), always take first 2 meaningful lines
            # This ensures we get both error message and any helpful context
            if len(non_empty_lines) >= 2:
                result = f"{non_empty_lines[0]} | {non_empty_lines[1]}"
            elif len(non_empty_lines) == 1:
                result = non_empty_lines[0]
            else:
                result = ""
        
        return result.strip()
    
    def execute_command_smart(self, cmd, suite):
        """Execute command with a simple timeout."""
        start_time = time.time()
        
        try:
            # Start the process
            process = subprocess.Popen(
                ['bash', '-c', cmd],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                preexec_fn=None
            )
            
            output_lines = []
            timeout_reached = False
            
            # Simple 30 second timeout for everything
            timeout_duration = 30.0
            
            # Execute with timeout
            try:
                stdout, _ = process.communicate(timeout=timeout_duration)
                output_lines = stdout.split('\n') if stdout else []
            except subprocess.TimeoutExpired:
                timeout_reached = True
                process.kill()
                try:
                    stdout, _ = process.communicate(timeout=1.0)
                    output_lines = stdout.split('\n') if stdout else []
                except:
                    output_lines = ["[TIMEOUT] Command stopped"]
            
            # Clean up process if still running
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=1.0)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            
            exit_code = process.returncode if process.returncode is not None else -1
            
            # Clean up output lines
            output_lines = [line.strip() for line in output_lines if line.strip()]
            
            if timeout_reached:
                output_lines.append("[TIMEOUT] Command stopped")
                
        except Exception as e:
            output_lines = [f"Error executing command: {str(e)}"]
            exit_code = -1
        
        # Calculate execution time
        execution_time = int((time.time() - start_time) * 1000)
        
        # Detect output type and format appropriately
        output_type = self.detect_output_type(output_lines)
        formatted_output = self.format_output_for_csv(output_lines, output_type, exit_code)
        
        return {
            'suite': suite,
            'command': cmd,
            'exit_code': exit_code,
            'output': formatted_output,
            'execution_time': execution_time
        }
    
    def load_commands(self):
        """Load commands from file."""
        if not Path(self.commands_file).exists():
            print(f"Commands file '{self.commands_file}' not found.")
            print("Please create a commands file with the following format:")
            print("# Test Suite Name")
            print("./ft_ping command1")
            print("./ft_ping command2")
            print("# Another Test Suite")
            print("./ft_ping command3")
            print("")
            print("Lines starting with # are test suite names")
            print("Other lines are commands to execute")
            return []
        
        commands = []
        current_suite = "Default"
        
        with open(self.commands_file, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                
                # Check if line is a test suite name (starts with #)
                if line.startswith('#'):
                    current_suite = line[1:].strip()
                    continue
                
                # Skip empty lines
                if not line:
                    continue
                
                commands.append((line, current_suite))
        
        return commands
    
    def save_results(self):
        """Save results to CSV file."""
        with open(self.output_file, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(['Test_Suite', 'Command', 'Exit_Code', 'Output', 'Execution_Time_ms'])
            
            for result in self.results:
                writer.writerow([
                    result['suite'],
                    result['command'],
                    result['exit_code'],
                    result['output'],
                    result['execution_time']
                ])
    
    def run(self):
        """Main execution method."""
        print(f"Collecting test results: {self.commands_file}")
        
        commands = self.load_commands()
        if not commands:
            return
        
        total_commands = len(commands)
        
        try:
            for i, (cmd, suite) in enumerate(commands, 1):
                print(f"[{i}/{total_commands}] Executing: {cmd}")
                try:
                    result = self.execute_command_smart(cmd, suite)
                    self.results.append(result)
                except KeyboardInterrupt:
                    print(f"\n⚠️  Interrupted during command: {cmd}")
                    print("Saving partial results...")
                    break
                except Exception as e:
                    print(f"Error with command '{cmd}': {e}")
                    # Add a failed result entry
                    self.results.append({
                        'suite': suite,
                        'command': cmd,
                        'exit_code': -1,
                        'output': f"Error: {str(e)}",
                        'execution_time': 0
                    })
        except KeyboardInterrupt:
            print("\n⚠️  Script interrupted by user")
            print("Saving partial results...")
        
        self.save_results()
        print(f"Saved {len(self.results)} test results to {self.output_file}")

def main():
    """Main function."""
    output_file = sys.argv[1] if len(sys.argv) > 1 else "test/ft_parser.results.csv"
    commands_file = sys.argv[2] if len(sys.argv) > 2 else "test/parser.test.txt"
    
    collector = CommandCollector(output_file, commands_file)
    collector.run()

if __name__ == "__main__":
    main()
