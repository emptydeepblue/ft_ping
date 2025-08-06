#ifndef FT_PING_H
#define FT_PING_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>

#define PING "ft_ping"
#define PARSE_ERROR_MISSING_DEST "usage error: Destination address required"
#define PARSE_ERROR_UNKNOWN_DEST "Name or service not known"
#define PARSE_ERROR_INVALID_OPT "invalid option -- "
#define PARSE_ERROR_ENCODING "Parameter string not correctly encoded"
#define DNS_ERROR_INVALID_DEST "Temporary failure in name resolution"
#define ALLOCATION_ERROR "memory allocation failed"
#define OPT_HELP "-?"
#define OPT_VERBOSE "-v"

typedef struct s_args {
    int opt_help;       /*-?*/
    int opt_verbose;    /*-v*/
    char *dest;         /* final validated destination */
} t_args;

int parse_args(char **, int, t_args *, char ***, int *);
int validate_destinations(char **, int, char **);

/*Utils*/
void print_help();
int has_non_ascii(const char *);
void *xcalloc(size_t size);
#endif

