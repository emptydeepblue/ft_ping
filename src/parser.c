/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:29:33 by everonel          #+#    #+#             */
/*   Updated: 2026/05/29 15:13:58 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ft_ping.h"

static void parse_short_options(t_args *, char *);
static void parse_long_options(t_args *, char *);

void parse_args(t_args *args, char **input, int input_size) {
    for (int i = 0; i < input_size; i++) {
        char *arg = input[i];
        if (arg[0] != '-' || arg[1] == '\0')
            add_host(args, arg);
        else if (arg[1] != '-')
            parse_short_options(args, arg);
        else if (arg[2] == '\0') {
            while (++i < input_size)
                add_host(args, input[i]);
            return;
        } else
            parse_long_options(args, arg);
    }
}

/* --- STATICS --- */
static void parse_short_options(t_args *args, char *arg) {
    for (int j = 1; arg[j] != '\0'; j++) {
        if (arg[j] == 'v')
            args->opt_verbose = 1;
        else if (arg[j] == '?')
            print_exit(args, HELP, EXIT_SUCCESS);
        else {
            print_error("invalid option -- '%c'", arg[j]);
            print_exit(args, ERROR | HINT, EXIT_USAGE);
        }
    }
}

static void parse_long_options(t_args *args, char *arg) {
    char *option = arg + 2;
    if (strcmp(option, "help") == 0)
        print_exit(args, HELP, EXIT_SUCCESS);
    
    if (option[0] == 'v' /*&& option[1] == '\0'*/) {
        print_error("option '%s' is ambiguous; possibilities: '-v' '-?'", arg);
        print_exit(args, ERROR, EXIT_USAGE);
    }
    
    print_error("unrecognized option '%s'", arg);
    
    print_exit(args, ERROR | HINT, EXIT_USAGE);
}
