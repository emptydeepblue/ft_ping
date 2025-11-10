/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dns.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/04 10:27:51 by everonel          #+#    #+#             */
/*   Updated: 2025/10/23 17:42:54 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

void validate_host_dns(t_host *host) {
    struct addrinfo hints, *result;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_flags = AI_CANONNAME;

    int status = getaddrinfo(host->name, NULL, &hints, &result);
    
    if (status != 0){
        print_error("unknown host\n");
        cleanup_and_exit(ERROR, EXIT_FAILURE);
    }

    memcpy(&host->addr, result->ai_addr, sizeof(host->addr));

    if (result->ai_canonname && strcmp(result->ai_canonname, host->name) != 0) {
        free(host->name);
        host->name = strdup(result->ai_canonname);
    }

    freeaddrinfo(result);
    host->addr.sin_port = 0;
}
