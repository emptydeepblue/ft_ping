/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:29:33 by everonel          #+#    #+#             */
/*   Updated: 2026/04/24 14:25:04 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

static void parse_options(t_args *, char **, int);
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
    validate_hosts(args);
}

static void parse_options(t_args *args, char **input, int input_size) {
    for (int i = 0; i < input_size; i++) {
        if (input[i][0] == '-' && input[i][1] != '\0') {
            if (input[i][1] == '?' || input[i][1] == 'h') {
                cleanup_and_exit(args, HELP, EXIT_SUCCESS);
            } else if (input[i][1] == 'v') {
                args->opt_verbose = 1;
                for (int j = 2; input[i][j] != '\0'; j++)
                    if (input[i][j] == '?' || input[i][j] == 'h')
                        cleanup_and_exit(args, HELP, EXIT_SUCCESS);
                    else if (input[i][j] != 'v') {
                        print_error("invalid option -- '%c'", input[i][j]);
                        cleanup_and_exit(args, ERROR | HINT, EXIT_USAGE);
                    }
            } else if (input[i][1] == '-') {
                if (input[i][2] == '\0') {
                    for (int j = i + 1; j < input_size; j++)
                        add_host(args, input[j]);
                    return;
                }
                if (strcmp(input[i] + 2, "help") == 0 || strcmp(input[i] + 2, "usage") == 0)
                    cleanup_and_exit(args, HELP, EXIT_SUCCESS);
                if (input[i][2] == 'v') {
                    print_error("option '%s' is ambiguous; possibilities: '-v' '-?'", input[i]);
                    cleanup_and_exit(args, ERROR, EXIT_USAGE);
                }
                print_error("unrecognized option '%s'", input[i]);
                cleanup_and_exit(args, ERROR | HINT, EXIT_USAGE);
            } else {
                print_error("invalid option -- '%c'", input[i][1]);
                cleanup_and_exit(args, ERROR | HINT, EXIT_USAGE);
            }
        } else {
            add_host(args, input[i]);
        }
    }
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
    t_host *new_host = xcalloc(args, sizeof(t_host));
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
