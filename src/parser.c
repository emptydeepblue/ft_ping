/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:29:33 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 10:52:44 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

static void parse_options(t_args *, char **, int);
static void parse_short_options(t_args *, char *);
static void parse_long_options(t_args *, char *);
static void validate_hosts(t_args *);
static void add_host(t_args *, char *);

void parse_args(t_args *args, char **input, int input_size) {
    args->opt_verbose = 0;
    args->hosts = NULL;

    parse_options(args, input, input_size);

    if (args->hosts == NULL) {
        print_error("missing host operand\n");
        cleanup_and_exit(args, ERROR | HINT, EXIT_USAGE);
    }
    validate_hosts(args);           // TODO: move in dns.c
}

static void parse_options(t_args *args, char **input, int input_size) {
    for (int i = 0; i < input_size; i++) {
        char *arg = input[i];
        if (arg[0] != '-' || arg[1] == '\0') {
            add_host(args, arg);
        } else if (arg[1] != '-') {
            parse_short_options(args, arg);
        } else if (arg[2] == '\0') {
            while (++i < input_size) {
                add_host(args, input[i]);
            }
            return;
        } else {
            parse_long_options(args, arg);
        }
    }
}

static void parse_short_options(t_args *args, char *arg) {
    for (int j = 1; arg[j] != '\0'; j++) {
        if (arg[j] == 'v'){
            args->opt_verbose = 1;
        } else if (arg[j] == '?') {
            cleanup_and_exit(args, HELP, EXIT_SUCCESS);
        } else {
            print_error("invalid option -- '%c'", arg[j]);
            cleanup_and_exit(args, ERROR | HINT, EXIT_USAGE);
        }
    }
}

static void parse_long_options(t_args *args, char *arg) {
    char *option = arg + 2;
    if (strcmp(option, "help") == 0 || strcmp(option, "usage") == 0) {
        cleanup_and_exit(args, HELP, EXIT_SUCCESS);
    } 
    if (option[0] == 'v' /*&& option[1] == '\0'*/) {
        print_error("option '%s' is ambiguous; possibilities: '-v' '-?'", arg);
        cleanup_and_exit(args, ERROR, EXIT_USAGE);
    }
    print_error("unrecognized option '%s'", arg);
    cleanup_and_exit(args, ERROR | HINT, EXIT_USAGE);
}

static void validate_hosts(t_args *args) {
    for (t_host *current = args->hosts; current != NULL; current = current->next) {
        if (has_non_ascii(current->name) || current->name[0] == '.' || current->name[0] == '\0') {
            print_error("unknown host\n");
            cleanup_and_exit(args, ERROR, EXIT_FAILURE);
        }
    }
}

static void add_host(t_args *args, char *host_name) {
    t_host *new_host = calloc(1, sizeof(t_host));
    if (!new_host) {
        print_error("memory allocation failed\n");
        cleanup_and_exit(args, ERROR, EXIT_FAILURE);
    }
    new_host->name = strdup(host_name);
    new_host->next = NULL;

    if (args->hosts == NULL) {
        args->hosts = new_host;
    } else {
        t_host *current = args->hosts;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_host;
    }
}
