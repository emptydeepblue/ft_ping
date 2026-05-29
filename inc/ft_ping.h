#ifndef FT_PING_H
#define FT_PING_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>

#define NAME "ping"
#define EXIT_USAGE 64

#define PACKET_SIZE 64
#define ICMP_ECHO_REQUEST 8
#define ICMP_HEADER_SIZE 8
#define ICMP_ECHO_REPLY 0

#define ICMP_DEST_UNREACH 3
#define ICMP_TIME_EXCEEDED 11
#define ICMP_NET_UNREACH 0
#define ICMP_HOST_UNREACH 1
#define ICMP_PORT_UNREACH 3
#define ICMP_EXC_TTL 0

typedef enum {
    NONE  = 0,
    ERROR = 1 << 0,
    HELP  = 1 << 1,
    HINT  = 1 << 2,
    STATS = 1 << 3,
} ExitFlags;

typedef struct s_host {
    char                *name;
    struct sockaddr_in  addr;
    struct s_host       *next;
} t_host;

typedef struct s_args {
    int                 opt_verbose;
    struct s_host       *hosts;
} t_args;

typedef struct s_icmp_packet {
    uint8_t             type;
    uint8_t             code;
    uint16_t            checksum;
    uint16_t            identifier;
    uint16_t            sequence;
    char                payload[PACKET_SIZE - ICMP_HEADER_SIZE];
} t_icmp_packet;

typedef struct s_ping_stats {
    char                *hostname;
    int                 packets_sent;
    int                 packets_recv;
    double              min_time;
    double              max_time;
    double              sum_squares;
    double              total_time;
} t_ping_stats;

/* Only global — signal handlers may only touch volatile sig_atomic_t
   objects. Everything else is stack-allocated in main() and threaded
   through the call graph. */
extern volatile sig_atomic_t g_keep_running;

/*parser.c*/
void            parse_args(t_args *, char **, int);
t_icmp_packet   *parse_reply(char *, ssize_t);

/*validator.c*/
void            validate_host(t_args *, t_host *);
int             validate_ownership(t_icmp_packet *, pid_t, int);

/*initializer.c*/
int             initialize_socket(t_args *);
t_ping_stats    initialize_stats(char *);
t_icmp_packet   initialize_packet(pid_t, int);

/*ping.c*/
void            ping(t_args *, t_host *);

/*print.c*/
void            print_error(const char *, ...);
void            print_hint(void);
void            print_help(void);
void            print_header(t_args *, t_host *, pid_t);
void            print_ping(t_icmp_packet *, char *, ssize_t, struct sockaddr_in, double);
void            print_icmp_error(t_args *, t_icmp_packet *, struct sockaddr_in *, int, pid_t);
void            print_stats(t_ping_stats *);

/*utils.c*/
void            add_host(t_args *, char *);
void            free_hosts(t_args *);
int             has_non_ascii(const char *);
void            cleanup_and_exit(t_args *, ExitFlags, int);
uint16_t        calculate_checksum(void *, int);
void            signal_handler(int);

#endif
