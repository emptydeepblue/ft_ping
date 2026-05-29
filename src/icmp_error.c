/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp_error.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 12:15:00 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 16:06:43 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

static void print_brief_error(t_icmp_packet *icmp);
static void print_ip_hdr_dump(struct iphdr *orig_ip);
static void print_ip_hdr_parsed(struct iphdr *orig_ip);

void print_icmp_error(t_args *args, t_icmp_packet *icmp,
                      struct sockaddr_in *from, int icmp_size, pid_t pid) {
    struct iphdr  *orig_ip   = (struct iphdr *)icmp->payload;
    t_icmp_packet *orig_icmp = (t_icmp_packet *)((char *)orig_ip + orig_ip->ihl * 4);
    uint16_t       orig_id   = ntohs(orig_icmp->identifier);
    uint16_t       orig_seq  = ntohs(orig_icmp->sequence);

    if (!args->opt_verbose || orig_id != (pid & 0xFFFF))
        return;

    fprintf(stderr, "%d bytes from %s: ", icmp_size, inet_ntoa(from->sin_addr));
    print_brief_error(icmp);
    print_ip_hdr_dump(orig_ip);
    print_ip_hdr_parsed(orig_ip);
    if (orig_ip->protocol == IPPROTO_ICMP) {
        int embedded_size = ntohs(orig_ip->tot_len) - orig_ip->ihl * 4;
        fprintf(stderr, "ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
            orig_icmp->type, orig_icmp->code, embedded_size, orig_id, orig_seq);
    }
}

static void print_brief_error(t_icmp_packet *icmp) {
    switch (icmp->type) {
        case ICMP_DEST_UNREACH:
            switch (icmp->code) {
                case ICMP_NET_UNREACH:  fprintf(stderr, "Destination Net Unreachable\n"); break;
                case ICMP_HOST_UNREACH: fprintf(stderr, "Destination Host Unreachable\n"); break;
                case ICMP_PORT_UNREACH: fprintf(stderr, "Destination Port Unreachable\n"); break;
                default: fprintf(stderr, "Dest Unreachable, Bad Code: %d\n", icmp->code); break;
            }
            break;
        case ICMP_TIME_EXCEEDED:
            switch (icmp->code) {
                case 0: fprintf(stderr, "Time to live exceeded\n"); break;
                case 1: fprintf(stderr, "Frag reassembly time exceeded\n"); break;
                default: fprintf(stderr, "Time exceeded, Bad Code: %d\n", icmp->code); break;
            }
            break;
        default:
            fprintf(stderr, "Unknown ICMP type: %d\n", icmp->type);
            break;
    }
}

static void print_ip_hdr_dump(struct iphdr *orig_ip) {
    uint8_t *bytes = (uint8_t *)orig_ip;
    int      len   = orig_ip->ihl * 4;

    fprintf(stderr, "IP Hdr Dump:\n ");
    for (int i = 0; i < len; i += 2)
        fprintf(stderr, "%02x%02x ", bytes[i], bytes[i + 1]);
    fprintf(stderr, "\n");
}

static void print_ip_hdr_parsed(struct iphdr *orig_ip) {
    uint16_t off_raw  = ntohs(orig_ip->frag_off);
    uint8_t  flags    = (off_raw >> 13) & 0x7;
    uint16_t frag_off = off_raw & 0x1fff;
    char     src[INET_ADDRSTRLEN];
    char     dst[INET_ADDRSTRLEN];

    strcpy(src, inet_ntoa(*(struct in_addr *)&orig_ip->saddr));
    strcpy(dst, inet_ntoa(*(struct in_addr *)&orig_ip->daddr));
    fprintf(stderr,
        "Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src       Dst     Data\n");
    fprintf(stderr, " %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s \n",
        orig_ip->version, orig_ip->ihl, orig_ip->tos,
        ntohs(orig_ip->tot_len), ntohs(orig_ip->id),
        flags, frag_off,
        orig_ip->ttl, orig_ip->protocol, ntohs(orig_ip->check),
        src, dst);
}
