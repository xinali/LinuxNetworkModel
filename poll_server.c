#include "utils.h"

#ifndef OPEN_MAX
#define OPEN_MAX 40960
#endif


int main(int argc, char **argv)
{
    int i, maxi, server_fd, client_fd, monitfd;
    int nready;
    ssize_t n;
    char buf_write[READ_MAX_SIZE] = SEND_2_CLIENT_MSG;
    char buf_read[WRITE_MAX_SIZE];
    memset(buf_read, 0, sizeof(buf_read));
    socklen_t clilen;
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in client_addr, server_addr;

    do
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            handle_error("socket");
            break;
        }
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(10086);

        if (-1 == bind(server_fd, (SA *)&server_addr, sizeof(server_addr)))
        {
            handle_error("bind");
            break;
        }

        if (-1 == listen(server_fd, LISTEN_BACKLOG))
        {
            handle_error("listen");
            break;
        }

        // index 0 存储服务端socket fd
        client[0].fd = server_fd;
        client[0].events = POLLRDNORM;
        for (i = 1; i < OPEN_MAX; i++)
            client[i].fd = -1; /* -1 indicates available entry */
        maxi = 0;              /* max index into client[] array */
                               /* end fig01 */
        for (;;)
        {
            nready = poll(client, maxi + 1, -1);

            // 客户端连接请求
            if (client[0].revents & POLLRDNORM)
            {
                clilen = sizeof(client_addr);
                client_fd = accept(server_fd, (SA *)&client_addr, &clilen);
                printf("connection from %s, port %d\n", 
                    inet_ntoa(client_addr.sin_addr), 
                    ntohs(client_addr.sin_port));
                
                // 加入监控集合
                for (i = 1; i < OPEN_MAX; i++)
                {
                    if (client[i].fd < 0)
                    {
                        client[i].fd = client_fd; /* save descriptor */
                        break;
                    }
                }
                if (i == OPEN_MAX)
                {
                    handle_error("too many clients");
                    break;
                }

                // 设置新fd events可读
                client[i].events = POLLRDNORM;
                if (i > maxi)
                    maxi = i; /* max index in client[] array */

                if (--nready <= 0)
                    continue; /* no more readable descriptors */
            }

            // 轮询所有使用中的事件
            for (i = 1; i <= maxi; i++)
            {
                if ((monitfd = client[i].fd) < 0)
                    continue;

                if (client[i].revents & (POLLRDNORM | POLLERR))
                {
                    if ((n = read(monitfd, buf_read, READ_MAX_SIZE)) < 0)
                    {
                        if (errno == ECONNRESET)
                        {
                            printf("client[%d] aborted connection\n", i);
                            close(monitfd);
                            client[i].fd = -1;
                        }
                        else
                            printf("read error");
                    }
                    else if (n == 0)
                    {
                        printf("client[%d] closed connection\n", i);
                        close(monitfd);
                        client[i].fd = -1;
                    }
                    else
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
    return 0;
}