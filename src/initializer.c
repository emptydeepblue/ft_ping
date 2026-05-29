/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initializer.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 11:11:17 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 11:30:31 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

int initialize_socket(t_args *args) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        if (errno == EPERM) {
            print_error("socket: Operation not permitted (need root)\n");
            cleanup_and_exit(args, ERROR, EXIT_FAILURE);
        } else {
            print_error("socket");
            cleanup_and_exit(args, ERROR, EXIT_FAILURE);
        }
    }
    struct timeval timeout = {1, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    return sockfd;
}

t_ping_stats initialize_stats(char *hostname) {
    t_ping_stats stats;

    stats.hostname = strdup(hostname);
    stats.packets_sent = 0;
    stats.packets_recv = 0;
    stats.min_time = DBL_MAX;
    stats.max_time = 0.0;
    stats.total_time = 0.0;
    stats.sum_squares = 0.0;
    return stats;
}

t_icmp_packet initialize_packet(pid_t pid, int sequence) {
    t_icmp_packet packet;
    memset(&packet, 0, sizeof(t_icmp_packet));
    packet.type = ICMP_ECHO_REQUEST;
    packet.code = 0;
    packet.checksum = 0;
    packet.identifier = htons(pid & 0xFFFF);
    packet.sequence = htons(sequence);
    // Fill data with something recognizable
    sprintf(packet.payload, "Hello from ping #%d", sequence);
    packet.checksum = calculate_checksum(&packet, sizeof(t_icmp_packet));
    return packet;
}
