/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 16:10:01 by everonel          #+#    #+#             */
/*   Updated: 2025/10/23 17:53:11 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

static int initialize_socket();
// static t_ping_stats initialize_stats();
static t_icmp_packet initialize_packet(pid_t, int);
static t_icmp_packet *parse_icmp_reply(char *recv, ssize_t recv_bytes) ;
static int check_packet_ownership(t_icmp_packet *reply, pid_t pid, int exp_seq);

void ping(t_host *host) {
    pid_t   pid = getpid(), sequence = 0;
    int     sockfd = initialize_socket();
    
    signal(SIGINT, signal_handler);
    print_header(host, pid);
    g_stats = initialize_stats(host->name); 
    
    while (keep_running) {
        struct timeval  send_time, recv_time;
        int             attempts = 0, found_our_reply = 0;
        t_icmp_packet   packet;
        
        packet = initialize_packet(pid, sequence);
        gettimeofday(&send_time, NULL);

        ssize_t sent = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&host->addr, sizeof(host->addr));    
        if (sent < 0) 
            continue;
        g_stats.packets_sent++;

        while (attempts < 50 && !found_our_reply) {
            char                recv_buffer[1024];
            struct sockaddr_in  recv_addr;
            socklen_t           addr_len = sizeof(recv_addr);
            
            ssize_t received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&recv_addr, &addr_len);
            if (received < 0)
                break;
            t_icmp_packet *reply = parse_icmp_reply(recv_buffer, received);

            if (reply != NULL) {
                if (check_packet_ownership(reply, pid, sequence)) { 
                    gettimeofday(&recv_time, NULL);
                    double rtt = (recv_time.tv_sec - send_time.tv_sec) * 1000.0 + 
                                (recv_time.tv_usec - send_time.tv_usec) / 1000.0;

                    print_ping(reply, recv_buffer, received, recv_addr, rtt);

                    g_stats.sum_squares += rtt * rtt;
                    if (rtt < g_stats.min_time) g_stats.min_time = rtt;
                    if (rtt > g_stats.max_time) g_stats.max_time = rtt;
                    g_stats.total_time += rtt;
                    g_stats.packets_recv++;
                    found_our_reply = 1;
                }
                // Check if this is an ICMP error message (in verbose mode)
                else if (reply->type == ICMP_DEST_UNREACH || reply->type == ICMP_TIME_EXCEEDED) {
                    print_icmp_error(reply, &recv_addr);
                }
            }
            attempts ++;
        }
        sequence++;
        if (!keep_running) {
            cleanup_and_exit(STATS, EXIT_SUCCESS);
        }
        sleep(1);
    }

    close(sockfd);
}

static int initialize_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        if (errno == EPERM) {
            print_error("socket: Operation not permitted (need root)\n");
            cleanup_and_exit(ERROR, EXIT_FAILURE);
        } else {
            print_error("socket");
            cleanup_and_exit(ERROR, EXIT_FAILURE);
        }
    }
    return sockfd;
}

static t_icmp_packet initialize_packet(pid_t pid, int sequence) {
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

static t_icmp_packet *parse_icmp_reply(char *recv, ssize_t recv_bytes) {
    if (recv_bytes < ICMP_HEADER_SIZE) 
        return NULL;
    
    struct iphdr    *ip_header = (struct iphdr *)recv;
    int             header_bytes = ip_header->ihl * 4;
    t_icmp_packet   *icmp_packet = NULL;

    if (recv_bytes < header_bytes + ICMP_HEADER_SIZE) 
        return NULL;

    icmp_packet = (t_icmp_packet *)(recv + header_bytes);
    return icmp_packet;
}

static int check_packet_ownership(t_icmp_packet *reply, pid_t pid, int exp_seq) {
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
    
// static t_ping_stats initialize_stats(char *hostname) {
//     t_ping_stats stats;
    
//     stats.hostname = hostname;
//     stats.packets_sent = 0;
//     stats.packets_recv = 0;
//     stats.min_time = DBL_MAX;
//     stats.max_time = 0.0;
//     stats.total_time = 0.0;
//     stats.sum_squares = 0.0;
//     return stats;
// }