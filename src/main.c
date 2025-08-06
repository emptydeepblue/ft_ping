#include "../inc/ft_ping.h"

int main (int ac, char **av) {
    t_args *args = xcalloc(sizeof(t_args));
    
    int res;
    char **dests = NULL;
    int dests_size = 0;
    if ((res = parse_args(av + 1, ac - 1, args, &dests, &dests_size)) != 0) {
        exit(res);
    }
    
    if (args->opt_help) {
        print_help();
        exit(2);
    }

    if ((res = validate_destinations(dests, dests_size, &args->dest)) != 0) {
        exit(res);
    }
    
    printf("Destination: %s\n", args->dest);
    printf("I'm pinging...\n");
    
    // Clean up
    return 0;
}

int parse_args(char **input, int input_size, t_args *args, char ***dests, int *dests_size) {

    if (input_size < 1) {
        fprintf(stderr, "%s: %s\n", PING, PARSE_ERROR_MISSING_DEST);
        return 1;
    }
    
    args->opt_help = 0;
    args->opt_verbose = 0;
    args->dest = NULL;
    *dests = xcalloc(sizeof(char*) * input_size);

    // Parse arguments
    for (int i = 0; i < input_size; i++) {
        if (input[i][0] == '-' && input[i][1] != '\0') {
            // Handle options that start with '-'
            if (input[i][1] == '?') {
                args->opt_help = 1;
            } else if (input[i][1] == 'v') {
                args->opt_verbose = 1;
                // Check if remaining characters are all 'v' (allow -vv, -vvv, etc.)
                for (int j = 2; input[i][j] != '\0'; j++) {
                    if (input[i][j] != 'v') {
                        fprintf(stderr, "%s: %s\'%c\'\n\n", PING, PARSE_ERROR_INVALID_OPT, input[i][j]);
                        print_help();
                        free(*dests);
                        return 2;
                    }
                }
            } else {
                // Invalid option
                fprintf(stderr, "%s: %s\'%c\'\n\n", PING, PARSE_ERROR_INVALID_OPT, input[i][1]);
                print_help();
                free(*dests);
                return 2;
            }
        } else {
            // Collect destination
            (*dests)[*dests_size] = input[i];
            (*dests_size)++;
        }
    }
    
    // Check if we have destinations when needed
    if (*dests_size <= 0 && !args->opt_help) {
        fprintf(stderr, "%s: %s\n", PING, PARSE_ERROR_MISSING_DEST);
        free(*dests);
        return 1;
    }
    
    // Validate all destinations (basic syntax check)
    for (int i = 0; i < *dests_size; i++) {
        if (has_non_ascii((*dests)[i])) {
            fprintf(stderr, "%s: %s: %s\n", PING, (*dests)[i], PARSE_ERROR_ENCODING);
            free(*dests);
            return 2;
        }
        // Allow '-' and other characters - let DNS validation handle them
        if (!isalnum((*dests)[i][0]) && strcmp((*dests)[i], "-") != 0) {
            fprintf(stderr, "%s: %s: %s\n", PING, (*dests)[i], PARSE_ERROR_UNKNOWN_DEST);
            free(*dests);
            return 2;
        }
    }
    
    return 0;
}

int validate_destinations(char **dests, int dests_count, char **final_dest) {
    char *last_valid_dest = NULL;
    int last_error_status = 0;
    char *last_failed_dest = NULL;
    int has_failed_dest = 0;

    for (int i = 0; i < dests_count; i++) {
        struct addrinfo hints, *result;
        
        // Initialize hints structure
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_RAW;

        // Try to resolve the hostname/IP
        int status = getaddrinfo(dests[i], NULL, &hints, &result);

        if (status != 0) {
            // DNS resolution failed - store the error info for later
            last_error_status = status;
            last_failed_dest = dests[i];
            has_failed_dest = 1;
            continue;
        }
        
        last_valid_dest = dests[i];
        freeaddrinfo(result);
    }
    
    // If ANY destination failed, report the last failure and exit
    if (has_failed_dest) {
        const char *error_msg;
        switch (last_error_status) {
            case EAI_NONAME:
                error_msg = PARSE_ERROR_UNKNOWN_DEST;
                break;
            case EAI_AGAIN:
                error_msg = DNS_ERROR_INVALID_DEST;
                break;
            case EAI_FAIL:
                error_msg = "Non-recoverable failure in name resolution";
                break;
            case EAI_MEMORY:
                error_msg = "Memory allocation failure";
                break;
            case EAI_SYSTEM:
                error_msg = "System error in name resolution";
                break;
            default:
                error_msg = gai_strerror(last_error_status);
                break;
        }

        fprintf(stderr, "%s: %s: %s\n", PING, last_failed_dest, error_msg);
        return 2;
    }
    
    // All destinations are valid - set the final destination to the last valid one
    *final_dest = last_valid_dest;
    return 0;
}