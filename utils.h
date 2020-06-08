#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>


#define LISTEN_BACKLOG      32
#define SERVER_PORT         10086
#define READ_MAX_SIZE       1024
#define WRITE_MAX_SIZE      1024

#define SEND_2_CLIENT_MSG   "Hello, message from server."
#define SEND_2_SERVER_MSG   "Hello, message from client."


typedef struct sockaddr SA;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

