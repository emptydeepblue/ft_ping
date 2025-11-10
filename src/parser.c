/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:29:33 by everonel          #+#    #+#             */
/*   Updated: 2025/10/23 17:42:42 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

static void parse_options(char **, int);
static void validate_hosts(void);
static void add_host(char *);

void parse_args(char **input, int input_size) {
    g_args->opt_verbose = 0;
    g_args->hosts = NULL;
    
    parse_options(input, input_size);
    
    if (g_args->hosts == NULL) {
        print_error("missing host operand\n");
        cleanup_and_exit(ERROR | HINT, EXIT_USAGE);
    }
    validate_hosts();
}

static void parse_options(char **input, int input_size) {
    for (int i = 0; i < input_size; i++) {
        if (input[i][0] == '-' && input[i][1] != '\0') {
            if (input[i][1] == '?') {
                cleanup_and_exit(HELP, EXIT_SUCCESS);
            } else if (input[i][1] == 'v') {
                g_args->opt_verbose = 1;
                for (int j = 2; input[i][j] != '\0'; j++)
                    if (input[i][j] == '?')
                        cleanup_and_exit(HELP, EXIT_SUCCESS);
                    else if (input[i][j] != 'v') {
                        print_error("invalid option -- '%c'", input[i][j]);
                        cleanup_and_exit(ERROR | HINT, EXIT_USAGE);
                    }
            } else if (input[i][1] == '-') {
                if (input[i][2] == 'v') {
                    print_error("option '%s' is ambiguous; possibilities: '-v' '-?'", input[i]);
                    cleanup_and_exit(ERROR, EXIT_USAGE);
                }
                print_error("unrecognized option '%s'", input[i]);
                cleanup_and_exit(ERROR | HINT, EXIT_USAGE);
            } else {
                print_error("invalid option -- '%c'", input[i][1]);
                cleanup_and_exit(ERROR | HINT, EXIT_USAGE);
            }
        } else {
            add_host(input[i]);
        }
    }
}

static void validate_hosts() {
    for (t_host *current = g_args->hosts; current != NULL; current = current->next) {
        if (has_non_ascii(current->name) || current->name[0] == '.' || current->name[0] == '\0') {
            print_error("unknown host\n");
            cleanup_and_exit(ERROR, EXIT_FAILURE);
        }
    }
}

static void add_host(char *host_name) {
    t_host *new_host = xcalloc(sizeof(t_host));
    new_host->name = strdup(host_name);
    new_host->next = NULL;
    
    if (g_args->hosts == NULL) {
        g_args->hosts = new_host;
    } else {
        t_host *current = g_args->hosts;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_host;
    }
}

