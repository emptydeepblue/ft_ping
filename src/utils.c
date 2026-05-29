/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:28:54 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 12:35:50 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

void add_host(t_args *args, char *host_name) {
    t_host *new_host = calloc(1, sizeof(t_host));
    
    if (!new_host) {
        print_error("memory allocation failed\n");
        print_exit(args, ERROR, EXIT_FAILURE);
    }
    new_host->name = strdup(host_name);
    new_host->next = NULL;

    if (args->hosts == NULL)
        args->hosts = new_host;
    else {
        t_host *current = args->hosts;
        while (current->next != NULL)
            current = current->next;
        current->next = new_host;
    }
}

void free_hosts(t_args *args) {
    if (!args) return;

    t_host *current = args->hosts;
    while (current) {
        t_host *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    args->hosts = NULL;
}

int has_non_ascii(const char *str) {
    if (!str) return 1;
    for (int i = 0; str[i]; i++)
        if ((unsigned char)str[i] > 127)
            return 1;
    return 0;
}

uint16_t calculate_checksum(void *data, int len) {
    uint16_t *buf = (uint16_t *)data;
    uint32_t sum = 0;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1) 
        sum += *(uint8_t *)buf;
    
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    
    return ~sum;
}

void signal_handler(int sig) {
    (void)sig;
    g_keep_running = 0;
}
