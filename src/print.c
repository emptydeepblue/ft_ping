/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:29:13 by everonel          #+#    #+#             */
/*   Updated: 2025/10/23 17:39:05 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"


void print_error(const char *format, ...) {
    va_list args;
    
    fprintf(stderr, "%s: ", NAME);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    if (format[strlen(format) - 1] != '\n') {
        fprintf(stderr, "\n");
    }
}

void print_hint() {
    printf("Try 'ping --help' or 'ping --usage' for more information.\n");
}

void print_help() {
    printf("Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n\n");
    printf("  -v, --verbose\n");
}

void print_header(t_host *host, pid_t pid) {
    printf("PING %s (%s): %d data bytes", host->name, inet_ntoa(host->addr.sin_addr), PACKET_SIZE - ICMP_HEADER_SIZE);
    if (g_args->opt_verbose) {
        printf(", id 0x%04x = %d", pid & 0xFFFF, pid & 0xFFFF);
    }
    printf("\n");
}

void print_ping(t_icmp_packet *reply, char *recv_buffer, ssize_t received, struct sockaddr_in recv_addr, double rtt) {
    uint16_t reply_seq = ntohs(reply->sequence);
    struct iphdr *ip_header = (struct iphdr *)recv_buffer;
    int ip_header_size = ip_header->ihl * 4;
    printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
            received - ip_header_size,
            inet_ntoa(recv_addr.sin_addr),
            reply_seq,
            ip_header->ttl,
            rtt
    );
}

void print_icmp_error(t_icmp_packet *icmp, struct sockaddr_in *from) {
    char *from_ip = inet_ntoa(from->sin_addr);
    
    switch (icmp->type) {
        case ICMP_DEST_UNREACH:
            fprintf(stderr, "From %s icmp_seq=%d Destination Unreachable", 
                from_ip, ntohs(icmp->sequence));
            
            if (g_args->opt_verbose) {
                switch (icmp->code) {
                    case ICMP_NET_UNREACH:
                        fprintf(stderr, ": Network Unreachable");
                        break;
                    case ICMP_HOST_UNREACH:
                        fprintf(stderr, ": Host Unreachable");
                        break;
                    case ICMP_PORT_UNREACH:
                        fprintf(stderr, ": Port Unreachable");
                        break;
                    default:
                        fprintf(stderr, ": Code %d", icmp->code);
                        break;
                }
            }
            fprintf(stderr, "\n");
            break;
            
        case ICMP_TIME_EXCEEDED:
            fprintf(stderr, "From %s icmp_seq=%d Time Exceeded", 
                from_ip, ntohs(icmp->sequence));
                
            if (g_args->opt_verbose && icmp->code == ICMP_EXC_TTL) {
                fprintf(stderr, ": TTL exceeded in transit");
            }
            fprintf(stderr, "\n");
            break;
            
        default:
            if (g_args->opt_verbose) {
                fprintf(stderr, "From %s icmp_seq=%d Unknown ICMP type %d code %d\n", 
                    from_ip, ntohs(icmp->sequence), icmp->type, icmp->code);
            }
            break;
    }
}


void print_stats() {
    printf("--- %s ping statistics ---\n", g_stats.hostname);
    free(g_stats.hostname);
    printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n",
           g_stats.packets_sent, g_stats.packets_recv,
           (g_stats.packets_sent > 0) ? (100.0 * (g_stats.packets_sent - g_stats.packets_recv) / g_stats.packets_sent) : 0.0);

    if (g_stats.packets_recv > 0) {
        double avg_rtt = g_stats.total_time / g_stats.packets_recv;
        double variance = (g_stats.sum_squares / g_stats.packets_recv) - (avg_rtt * avg_rtt);
        double stddev = sqrt(variance);
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
               g_stats.min_time, avg_rtt, g_stats.max_time, stddev);
    }
}