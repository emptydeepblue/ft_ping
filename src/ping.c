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

static int initialize_socket(t_args *args);
static t_icmp_packet initialize_packet(pid_t, int);
static t_icmp_packet *parse_icmp_reply(char *recv, ssize_t recv_bytes);
static int check_packet_ownership(t_icmp_packet *reply, pid_t pid, int exp_seq);

void ping(t_args *args, t_host *host) {
    pid_t        pid = getpid(), sequence = 0;
    int          sockfd = initialize_socket(args);
    t_ping_stats stats;

    signal(SIGINT, signal_handler);
    print_header(args, host, pid);
    stats = initialize_stats(host->name);

    while (g_keep_running) {
        struct timeval  send_time, recv_time;
        int             attempts = 0, found_our_reply = 0;
        t_icmp_packet   packet;

        packet = initialize_packet(pid, sequence);
        gettimeofday(&send_time, NULL);

        ssize_t sent = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&host->addr, sizeof(host->addr));
        if (sent < 0)
            continue;
        stats.packets_sent++;

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

                    stats.sum_squares += rtt * rtt;
                    if (rtt < stats.min_time) stats.min_time = rtt;
                    if (rtt > stats.max_time) stats.max_time = rtt;
                    stats.total_time += rtt;
                    stats.packets_recv++;
                    found_our_reply = 1;
                }
                else if (reply->type == ICMP_DEST_UNREACH || reply->type == ICMP_TIME_EXCEEDED) {
                    struct iphdr *outer_ip = (struct iphdr *)recv_buffer;
                    int icmp_size = received - outer_ip->ihl * 4;
                    print_icmp_error(args, reply, &recv_addr, icmp_size, pid);
                }
            }
            attempts ++;
        }
        sequence++;
        if (!g_keep_running)
            break;
        sleep(1);
    }

    print_stats(&stats);
    close(sockfd);
}

static int initialize_socket(t_args *args) {
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
