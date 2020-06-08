#include "utils.h"

#define SEND_MSG "hello world"

int main()
{
    int server_fd = -1;
    do 
    {
        struct sockaddr_in server_addr, client_addr;
        unsigned char client_host[256];
        memset((void*)client_host, 0, sizeof(client_host));
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) 
        {
            handle_error("socket");
            break;
        }
        memset((void*)&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET; /* ipv4 tcp packet */
        server_addr.sin_port = htons(SERVER_PORT); /* convert to network byte order */
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(server_fd, (SA*)&server_addr, sizeof(struct sockaddr_in)) == -1)
        {
            handle_error("bind");
            break;
        }

        if (listen(server_fd, 32) == -1)
        {
            handle_error("listen");
            break;
        }
        printf("waiting for connect to server...\n");
        int client_fd;
        int client_addr_len = sizeof(struct sockaddr_in);
        if ((client_fd = accept(server_fd, (SA*)&client_addr, 
                                    (socklen_t*)&client_addr_len)) == -1)
        {
            handle_error("accept");
            break;
        }
        printf("connection from %s, port %d\n", 
                    inet_ntoa(client_addr.sin_addr), 
                    ntohs(client_addr.sin_port));
        write(client_fd, SEND_MSG, sizeof(SEND_MSG));
        
    } while (0);
    
    if (server_fd != -1)
        close(server_fd);

    return 0;
}