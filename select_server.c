#include "utils.h"


int main(int argc, char **argv)
{
    int i, maxi, maxfd, server_fd, client_fd, monitfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf_read[READ_MAX_SIZE];
    memset((void*)buf_read, 0, sizeof(buf_read));
    char buf_write[WRITE_MAX_SIZE] = SEND_2_CLIENT_MSG;
    socklen_t clilen;
    struct sockaddr_in client_addr, server_addr;

    do
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            handle_error("socket");
            break;
        }

        memset((void*)&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(SERVER_PORT);

        if (-1 == bind(server_fd, 
                    (SA*)&server_addr, 
                    sizeof(server_addr)))
        {
            handle_error("bind");
            break;
        }

        if (-1 == listen(server_fd, LISTEN_BACKLOG))
        {
            handle_error("listen");
            break;
        }

        maxfd = server_fd; 
        maxi = -1;
        for (i = 0; i < FD_SETSIZE; i++)
            client[i] = -1; 
        FD_ZERO(&allset);
        FD_SET(server_fd, &allset);

        for (;;)
        {
            rset = allset;
            nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

            if (FD_ISSET(server_fd, &rset))
            {
                clilen = sizeof(client_addr);
                client_fd = accept(server_fd, (SA*)&client_addr, &clilen);

                printf("connection from %s, port %d\n", 
                    inet_ntoa(client_addr.sin_addr), 
                    ntohs(client_addr.sin_port));

                for (i = 0; i < FD_SETSIZE; i++)
                {
                    if (client[i] < 0)
                    {
                        client[i] = client_fd;
                        break;
                    }
                }
                if (i == FD_SETSIZE)
                {
                    handle_error("too many clients");
                    break;
                }

                FD_SET(client_fd, &allset); 
                if (client_fd > maxfd)
                    maxfd = client_fd; 
                if (i > maxi)
                    maxi = i; 

                if (--nready <= 0)
                    continue; /* no more readable descriptors */
            }

            for (i = 0; i <= maxi; i++)
            {
                if ((monitfd = client[i]) < 0)
                    continue;
                if (FD_ISSET(monitfd, &rset))
                {
                    // 请求关闭连接
                    if ((n = read(monitfd, buf_read, READ_MAX_SIZE)) == 0)
                    {
                        printf("client[%d] aborted connection\n", i);
                        close(monitfd);
                        client[i] = -1;
                    }
                    // 发生错误
                    if (n < 0)
                    {
                        printf("client[%d] closed connection\n", i);
                        close(monitfd);
                        client[i] = -1;
                        handle_error("read");
                        break;
                    }
                    else // 发送数据给客户端
                    {
                        printf("Client: %s\n", buf_read);
                        write(monitfd, buf_write, strlen(buf_write));
                    }

                    if (--nready <= 0)
                        break;
                }
            }
        }
    } while (0);
}
