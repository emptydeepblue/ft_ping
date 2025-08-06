#include "../inc/ft_ping.h"

void *xcalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "%s: %s\n", PING, ALLOCATION_ERROR);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int has_non_ascii(const char *str) {
    while (*str) {
        if ((unsigned char)*str > 127) {
            return 1;
        }
        str++;
    }
    return 0;
}

void print_help() {
    printf("Usage\n  ft_ping [options] <destination>\n\nOptions:\n  <destination>      dns name or ip address\n  -v                 verbose output\n");
    fflush(stdout);
}