/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validator.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/04 10:27:51 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 12:07:46 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

void validate_host(t_args *args, t_host *host) {
    if (has_non_ascii(host->name) || host->name[0] == '.' || host->name[0] == '\0') {
        print_error("unknown host\n");
        cleanup_and_exit(args, ERROR, EXIT_FAILURE);
    }
    struct addrinfo hints, *result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_flags = AI_CANONNAME;

    int status = getaddrinfo(host->name, NULL, &hints, &result);
    if (status != 0){
        print_error("unknown host\n");
        cleanup_and_exit(args, ERROR, EXIT_FAILURE);
    }

    memcpy(&host->addr, result->ai_addr, sizeof(host->addr));
    if (result->ai_canonname && strcmp(result->ai_canonname, host->name) != 0) {
        free(host->name);
        host->name = strdup(result->ai_canonname);
    }

    freeaddrinfo(result);
    host->addr.sin_port = 0;
}

int validate_ownership(t_icmp_packet *reply, pid_t pid, int exp_seq) {
    if (reply->type != ICMP_ECHO_REPLY)
        return 0;

    uint16_t reply_id = ntohs(reply->identifier);
    if (reply_id != (pid & 0xFFFF))
        return 0;

    uint16_t reply_seq = ntohs(reply->sequence);
    if (reply_seq != exp_seq)
        return 0;

    return 1;
}