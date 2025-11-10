/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:28:32 by everonel          #+#    #+#             */
/*   Updated: 2025/10/23 16:02:36 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

// Global variable definitions (declared in header with extern)

/* core variables */
t_args *g_args = NULL;
volatile int keep_running = 1;
t_ping_stats g_stats = {0};

/* statistics handling */
// static double sum_squares = 0.0;


int main (int ac, char **av) {
    g_args = xcalloc(sizeof(t_args));
    // Parse command line arguments
    // This will fill g_args with options and hosts with destination addresses
    // If there are parsing errors, it will print an error and exit
    // If help is requested, it will print help and exit
    parse_args(av + 1, ac - 1);
    for (t_host *host = g_args->hosts; host != NULL; host = host->next) {
        // DNS validation of destinations
        // If any destination is invalid, it will print an error and exit
        // If all are valid, it will set g_args->hosts->addr to the resolved address
        validate_host_dns(host);
        if (host->addr.sin_family != AF_INET) {
            printf("Host %s is not a valid IPv4 address.\n", host->name);
            continue;
        }
        // Ping each destination
        // Pass the specific host to ping, not all hosts
        ping(host);
    }
    // free_hosts();
    // free(g_args);
    return 0;
}
