#include "utils.h"
#include <signal.h>

int server_fd;

void do_sigio(int sig)
{
    char buf_read[READ_MAX_SIZE];
    memset((void *)buf_read, 0, sizeof(buf_read));
    struct sockaddr_in client_addr; 
    unsigned int clntLen; 
    int recvMsgSize; 
    do 
    {
        clntLen = sizeof(client_addr);
        if ((recvMsgSize = recvfrom(server_fd, 
                                buf_read, 
                                READ_MAX_SIZE, 
                                MSG_WAITALL,
                                (SA *)&client_addr, 
                                &clntLen)) < 0)
        {
            if (errno != EWOULDBLOCK)
            {
                handle_error("recvfrom");
                break;
            }
        }
        else
        {
            printf("connection from %s, port %d, data: %s\n", 
                    inet_ntoa(client_addr.sin_addr), 
                    ntohs(client_addr.sin_port), buf_read);
            if (sendto(server_fd, 
                       SEND_2_CLIENT_MSG, 
                       strlen(SEND_2_CLIENT_MSG), 
                       0, 
                       (SA *)&client_addr, 
                       sizeof(client_addr)) != strlen(SEND_2_CLIENT_MSG))
            {
                handle_error("sendto");
                break;
            }
        }
    } while (0);
}

int main()
{
    server_fd = -1;
    do
    {
        struct sockaddr_in server_addr;
        server_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_fd == -1)
        {
            handle_error("socket");
            break;
        }

        bzero((char *)&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (-1 == bind(server_fd, (SA *)&server_addr, sizeof(server_addr)))
        {
            handle_error("bind");
            break;
        }

        struct sigaction sigio_action;
        memset(&sigio_action, 0, sizeof(sigio_action));
        sigio_action.sa_flags = 0;
        sigio_action.sa_handler = do_sigio;
        if (sigfillset(&sigio_action.sa_mask) < 0) 
        {
            handle_error("sigfillset");
            break;
        }
        sigaction(SIGIO, &sigio_action, NULL);
        if (-1 == fcntl(server_fd, F_SETOWN, getpid()))
        {
            handle_error("fcntl_setdown");
            break;
        }
        
        int flags;
        flags = fcntl(server_fd, F_GETFL, 0);
        if (flags == -1)
        {
            handle_error("fcntl_getfl");
            break;
        }
        flags |= O_ASYNC | O_NONBLOCK;
        fcntl(server_fd, F_SETFL, flags);
        for (; ;)
        {
            printf("waiting ...\n");
            sleep(3);
        }
        close(server_fd);
    } while (0);
    return 0;
}