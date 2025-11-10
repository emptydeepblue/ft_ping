/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:28:54 by everonel          #+#    #+#             */
/*   Updated: 2025/10/23 17:59:08 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"


void *xcalloc(size_t size) {
    void *ptr = calloc(1, size);
    if (!ptr) {
        print_error("memory allocation failed\n");
        cleanup_and_exit(ERROR, EXIT_FAILURE);
    }
    return ptr;
}

void free_hosts() {
    if (!g_args) return;
    
    t_host *current = g_args->hosts;
    while (current) {
        t_host *next = current->next;
        free(current);
        current = next;
    }
    g_args->hosts = NULL;
}

int has_non_ascii(const char *str) {
    if (!str) return 1;
    for (int i = 0; str[i]; i++)
        if ((unsigned char)str[i] > 127)
            return 1;
    return 0;
}

void cleanup_and_exit(ExitFlags flags, int exit_code) {
    if (flags & HELP) {
        print_help();
    }
    if (flags & HINT) {
        print_hint();
    }
    free_hosts();
    free(g_args);
    exit(exit_code);
}

void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;

    for (t_host *tmp = g_args->hosts; tmp != NULL; tmp = tmp->next) {
        print_stats();
        g_stats = initialize_stats(tmp->name);
        g_stats.packets_sent = 1;
    }
    free_hosts();
    free(g_args);
    exit(EXIT_SUCCESS);
}

uint16_t calculate_checksum(void *data, int len) {
    uint16_t *buf = (uint16_t *)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    
    if (len == 1) {
        sum += *(uint8_t *)buf;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

t_ping_stats initialize_stats(char *hostname) {
    t_ping_stats stats;
    
    stats.hostname = hostname;
    stats.packets_sent = 0;
    stats.packets_recv = 0;
    stats.min_time = DBL_MAX;
    stats.max_time = 0.0;
    stats.total_time = 0.0;
    stats.sum_squares = 0.0;
    return stats;
}