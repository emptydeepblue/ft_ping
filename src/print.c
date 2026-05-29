/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:29:13 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 11:44:12 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

void print_header(t_args *args, t_host *host, pid_t pid) {
    printf("PING %s (%s): %d data bytes", host->name, inet_ntoa(host->addr.sin_addr), PACKET_SIZE - ICMP_HEADER_SIZE);
    if (args->opt_verbose)
        printf(", id 0x%04x = %d", pid & 0xFFFF, pid & 0xFFFF);
    printf("\n");
}

void print_ping(t_icmp_packet *reply, char *recv_buffer, ssize_t received, struct sockaddr_in recv_addr, double rtt) {
    struct iphdr *ip_header = (struct iphdr *)recv_buffer;
    int           ip_header_size = ip_header->ihl * 4;

    printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
        received - ip_header_size,
        inet_ntoa(recv_addr.sin_addr),
        ntohs(reply->sequence),
        ip_header->ttl,
        rtt);
}

void print_stats(t_ping_stats *stats) {
    printf("--- %s ping statistics ---\n", stats->hostname);
    free(stats->hostname);
    stats->hostname = NULL;
    printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n",
        stats->packets_sent, stats->packets_recv,
        (stats->packets_sent > 0) ? (100.0 * (stats->packets_sent - stats->packets_recv) / stats->packets_sent) : 0.0);

    if (stats->packets_recv > 0) {
        double avg_rtt  = stats->total_time / stats->packets_recv;
        double variance = (stats->sum_squares / stats->packets_recv) - (avg_rtt * avg_rtt);
        double stddev   = sqrt(fmax(0.0, variance));
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
            stats->min_time, avg_rtt, stats->max_time, stddev);
    }
}
