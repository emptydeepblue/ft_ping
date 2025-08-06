#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 8192

typedef struct {
    char *description;
    char *args[10];
    char *expected_output;
    int expected_exit;
} TestCase;

int run_test(TestCase t) {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();
    if (pid == 0) {
        // child
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp("./ft_ping", t.args);
        perror("execvp failed");
        exit(255);
    } else {
        // parent
        close(pipefd[1]);
        char buffer[BUF_SIZE];
        ssize_t count = read(pipefd[0], buffer, BUF_SIZE - 1);
        if (count < 0) count = 0;
        buffer[count] = '\0';
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 255;
        int success = strstr(buffer, t.expected_output) != NULL && exit_code == t.expected_exit;

        printf("[%s] %s\n", t.description, success ? "✅ PASSED" : "❌ FAILED");
        if (!success) {
            printf("    Expected exit: %d | Got: %d\n", t.expected_exit, exit_code);
            printf("    Expected output (part): \"%s\"\n", t.expected_output);
            printf("    Actual output:\n%s\n", buffer);
        }

        return success;
    }
}

int main() {
    TestCase tests[] = {
        // 1 - Parsing error
        {"1. Parsing: [./ft_ping ]", {"ft_ping", NULL}, "Destination address required", 1},
        // 2 - Malformed domains (unknown service) starting with dots, commas, or other invalid characters
        {"2. Parsing: [./ft_ping .com]", {"ft_ping", ".com", NULL}, "Name or service not known", 2},
        {"3. Parsing: [./ft_ping ,com]", {"ft_ping", ",com", NULL}, "Name or service not known", 2},
        {"4. Parsing: [./ft_ping +com]", {"ft_ping", "+com", NULL}, "Name or service not known", 2},
        {"5. Parsing: [./ft_ping '']", {"ft_ping", "", NULL}, "Name or service not known", 2},
        {"6. Parsing: [./ft_ping 💀.com]", {"ft_ping", "💀.com", NULL}, "Parameter string not correctly encoded", 2},
        // 8 - Weird dash
        {"7. Parsing: [./ft_ping -]", {"ft_ping", "-", NULL}, "Name or service not known", 2},
        {"8. Parsing: [./ft_ping 127.0.0.1 -]", {"ft_ping", "127.0.0.1", "-", NULL}, "Name or service not known", 2},
        // 10 - Invalid option
        {"9. Parsing: [./ft_ping -e]", {"ft_ping", "-e", NULL}, "invalid option", 2},
        {"10. Parsing: [./ft_ping --e]", {"ft_ping", "--e", NULL}, "invalid option", 2},
        {"11. Parsing: [./ft_ping -e 127.0.0.1]", {"ft_ping", "-e", "127.0.0.1", NULL}, "invalid option", 2},
        {"12. Parsing: [./ft_ping 127.0.0.1 -e]", {"ft_ping", "127.0.0.1", "-e", NULL}, "invalid option", 2},
        // 14 - Verbose without address
        {"13. Parsing: [./ft_ping -v]", {"ft_ping", "-v", NULL}, "Destination address required", 1},
        {"14. Parsing: [./ft_ping -vv]", {"ft_ping", "-vv", NULL}, "Destination address required", 1},
        
        // 3 - DNS resolution failures
        {"7. DNS fail: [./ft_ping q]", {"ft_ping", "q", NULL}, "Temporary failure in name resolution", 2},
        {"8. DNS fail: [./ft_ping google.c0m]", {"ft_ping", "google.c0m", NULL}, "Name or service not known", 2},
        {"9. DNS fail: [./ft_ping 127.0.0.1extra]", {"ft_ping", "127.0.0.1extra", NULL}, "Name or service not known", 2},
        // 6 - Multiple destinations with failure
        {"1. DNS fail: [./ft_ping test 127.0.0.1]", {"ft_ping", "test", "127.0.0.1", NULL}, "Temporary failure in name resolution", 2},
        {"2. DNS fail: [./ft_ping 127.0.0.1 test]", {"ft_ping", "127.0.0.1", "test", NULL}, "Temporary failure in name resolution", 2},
        {"7. DNS fail: [./ft_ping - 127.0.0.1]", {"ft_ping", "-", "127.0.0.1", NULL}, "Temporary failure in name resolution", 2},
        // 15 - Verbose with invalid dest
        {"3. DNS fail: [./ft_ping 127.0.0.1 test -v]", {"ft_ping", "127.0.0.1", "test", "-v", NULL}, "Temporary failure in name resolution", 2},
        
        // // 7 - Multiple destinations, valid but silent
        // {"3. 100\% packet loss: [./ft_ping 127.0.0.1 127.0.0.2]", {"ft_ping", "127.0.0.1", "127.0.0.2", NULL}, "packet loss", 0},
        // // 4 - Weird IPs but syntactically valid
        // {"10. 100\% packet loss: [./ft_ping 1]", {"ft_ping", "1", NULL}, "packet loss", 0},
        // {"11. 100\% packet loss: [./ft_ping 999.999.999.999]", {"ft_ping", "999.999.999.999", NULL}, "packet loss", 0},
        // // 16 - Verbose and multiple
        // {"4. 100\% packet loss: [./ft_ping -v 127.0.0.1 127.0.0.2]", {"ft_ping", "-v", "127.0.0.1", "127.0.0.2", NULL}, "packet loss", 0},

        // // 11 - Help
        {"1. Valid: [./ft_ping -?]", {"ft_ping", "-?", NULL}, "Usage", 2},
        {"2. Valid: [./ft_ping -??]", {"ft_ping", "-??", NULL}, "Usage", 2},
        // 12 - Help with args
        {"3. Valid: [./ft_ping 999.999.999.999 -?]", {"ft_ping", "999.999.999.999", "-?", NULL}, "Usage", 2},
        {"4. Valid: [./ft_ping 127.0.0.1 -?]", {"ft_ping", "127.0.0.1", "-?", NULL}, "Usage", 2},
        {"5. Valid: [./ft_ping test 127.0.0.1 test -? test]", {"ft_ping", "test", "127.0.0.1", "test", "-?", "test", NULL}, "Usage", 2},
        // 13 - Help variant
        {"6. Valid: [./ft_ping -?test]", {"ft_ping", "-?test", NULL}, "Usage", 2},
        // // 18 - Verbose + help
        {"1. Valid: [./ft_ping -? -v]", {"ft_ping", "-?", "-v", NULL}, "Usage", 2},
        {"2. Valid: [./ft_ping -v -?]", {"ft_ping", "-v", "-?", NULL}, "Usage", 2},
        // 19 - Weird help forms
        {"3. Valid: [./ft_ping 127.0.0.1 -v?]", {"ft_ping", "127.0.0.1", "-v?", NULL}, "Usage", 2},
        {"4. Valid: [./ft_ping 127.0.0.1 -?v]", {"ft_ping", "127.0.0.1", "-?v", NULL}, "Usage", 2},
        // // 5 - Valid pings
        // {"12. Valid: [./ft_ping 127.0.0.1]", {"ft_ping", "127.0.0.1", NULL}, "bytes from", 0},
        // {"13. Valid: [./ft_ping google.com]", {"ft_ping", "google.com", NULL}, "bytes from", 0},
        // // 17 - Verbose valid
        // {"5. Valid: [./ft_ping -v 127.0.0.1]", {"ft_ping", "-v", "127.0.0.1", NULL}, "bytes from", 0},
        // {"6. Valid: [./ft_ping -v google.com]", {"ft_ping", "-v", "google.com", NULL}, "bytes from", 0},
        // {"7. Valid: [./ft_ping -vv 127.0.0.1]", {"ft_ping", "-vv", "127.0.0.1", NULL}, "bytes from", 0},
        // {"8. Valid: [./ft_ping -v -v google.com]", {"ft_ping", "-v", "-v", "google.com", NULL}, "bytes from", 0},
    };

    int total = sizeof(tests) / sizeof(TestCase);
    int passed = 0;

    for (int i = 0; i < total; i++) {
        passed += run_test(tests[i]);
    }

    printf("\n%d/%d tests passed\n", passed, total);
    return passed == total ? 0 : 1;
}
