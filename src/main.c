/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:28:32 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 11:22:08 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

volatile sig_atomic_t g_keep_running = 1;

int main(int ac, char **av) {
    t_args args = {0};
    args.opt_verbose = 0;
    args.hosts = NULL;
    
    parse_args(&args, av + 1, ac - 1);
    
    if (args.hosts == NULL) {
        print_error("missing host operand\n");
        cleanup_and_exit(&args, ERROR | HINT, EXIT_USAGE);
    }
    
    for (t_host *host = args.hosts; host != NULL; host = host->next) {
        validate_host(&args, host);
        if (host->addr.sin_family != AF_INET) {
            printf("Host %s is not a valid IPv4 address.\n", host->name);
            continue;
        }
        ping(&args, host);
    }
    free_hosts(&args);
    return 0;
}
