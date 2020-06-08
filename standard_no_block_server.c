#include "utils.h"


int main()
{
    int server_fd, client_fd;           
    struct sockaddr_in server_addr, client_addr; 
    int sin_size;
    char buf_read[255];
    memset(buf_read, 0, sizeof(buf_read));
    int last_fd; 

    do
    {
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            handle_error("socket");
            break;
        }
        last_fd = server_fd;
        server_addr.sin_family = AF_INET; 
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(server_addr.sin_zero), 8); 

        if (bind(server_fd, (SA *)&server_addr, sizeof(SA)) == -1)
        {
            handle_error("bind");
            break;
        }
        if (listen(server_fd, LISTEN_BACKLOG) == -1)
        {
            handle_error("listen");
            break;
        }
        if ((client_fd = accept(server_fd, 
                            (SA *)&client_addr,
                             (socklen_t*)&sin_size)) == -1)
        {
            handle_error("accept");
            break;
        }

        fcntl(last_fd, F_SETFL, O_NONBLOCK); 
        fcntl(client_fd, F_SETFL, O_NONBLOCK);  
        for (; ;)
        {
            for (int i = server_fd; i <= last_fd; i++)
            {
                printf("Round number %d\n", i);
                if (i == server_fd)
                {
                    sin_size = sizeof(struct sockaddr_in);
                    if ((client_fd = accept(server_fd, (SA *)&client_addr,
                                        (socklen_t*)&sin_size)) == -1)
                    {
                        handle_error("accept");
                        continue;
                    }
                    printf("server: got connection from %s\n",
                           inet_ntoa(client_addr.sin_addr));
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    last_fd = client_fd;
                }
                else
                {
                    ssize_t recv_size = read(client_fd, buf_read, READ_MAX_SIZE);
                    if (recv_size < 0)
                    {
                        handle_error("recv");
                        break;
                    }
                    if (recv_size == 0)
                    {
                        close(client_fd);
                        continue;
                    }
                    else
                    {
                        buf_read[recv_size] = '\0';
                        printf("The string is: %s \n", buf_read);
                        if (write(client_fd, SEND_2_CLIENT_MSG, strlen(SEND_2_CLIENT_MSG)) == -1)
                        {
                            handle_error("send");
                            continue;
                        }
                    }
                }
            }
        }
    } while (0);
}