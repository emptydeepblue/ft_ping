/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 12:15:00 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 16:02:33 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

void print_error(const char *format, ...) {
    va_list ap;
    size_t  len = strlen(format);
    
    fprintf(stderr, "%s: ", NAME);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    if (len == 0 || format[len - 1] != '\n')
    fprintf(stderr, "\n");
}

static void print_hint(void);
static void print_help(void);

void print_exit(t_args *args, ExitFlags flags, int exit_code) {
    if (flags & HELP)
        print_help();
    if (flags & HINT)
        print_hint();
    free_hosts(args);
    exit(exit_code);
}

static void print_hint(void) {
    fprintf(stderr, "Try 'ping --help' for more information.\n");
}

static void print_help(void) {
    printf("Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n\n");
    printf("  -v, --verbose              verbose output\n\n");
    printf(" Options valid for --echo requests:\n\n");
    printf("  -?, --help                 give this help list\n\n");
    printf("Mandatory or optional arguments to long options are also mandatory or optional\nfor any corresponding short options.\n\n");
    printf("Options marked with (root only) are available only to superuser.\n\n");
    printf("Report bugs to <everonel@student.42firenze.it>.\n");
}