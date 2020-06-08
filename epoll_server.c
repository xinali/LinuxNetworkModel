#include "utils.h"

int main()
{
    int server_fd = -1;
    do
    {
        int client_fd, sockfd, epfd, nfds;
        ssize_t n;
        char buf_write[READ_MAX_SIZE] = SEND_2_CLIENT_MSG;
        char buf_read[WRITE_MAX_SIZE];
        memset(buf_read, 0, sizeof(buf_read));

        socklen_t clilen;
        //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
        struct epoll_event ev, events[20];
        //生成用于处理accept的epoll专用的文件描述符
        epfd = epoll_create(256);
        struct sockaddr_in client_addr;
        struct sockaddr_in server_addr;
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        // 设置为非阻塞
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
        if (server_fd == -1)
        {
            handle_error("socket");
            break;
        }
        //把socket设置为非阻塞方式
        //setnonblocking(server_fd);

        //设置与要处理的事件相关的文件描述符
        ev.data.fd = server_fd;
        //设置要处理的事件类型
        ev.events = EPOLLIN | EPOLLET;
        //注册epoll事件
        epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

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

        for (;;)
        {
            //等待epoll事件的发生
            nfds = epoll_wait(epfd, events, 20, 500);
            if (nfds == -1)
            {
                handle_error("epoll_wait");
                break;
            }
            //处理所发生的所有事件
            for (int i = 0; i < nfds; ++i)
            {
                // server_fd 事件
                if (events[i].data.fd == server_fd)
                {
                    client_fd = accept(server_fd, (SA *)&client_addr, &clilen);
                    if (client_fd == -1)
                    {
                        handle_error("accept");
                        break;
                    }
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    printf("connection from %s, port %d\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                    //设置用于读操作的文件描述符
                    ev.data.fd = client_fd;
                    //设置用于注测的读操作事件
                    ev.events = EPOLLIN | EPOLLET;
                    //注册ev
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                }
                else if (events[i].events & EPOLLIN) //已连接用户，并且收到数据，那么进行读入。
                {
                    if ((sockfd = events[i].data.fd) < 0)
                        continue;
                    if ((n = read(sockfd, buf_read, READ_MAX_SIZE)) < 0)
                    {
                        // 删除sockfd
                        epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
                        if (errno == ECONNRESET)
                        {
                            close(sockfd);
                            events[i].data.fd = -1;
                        }
                        else
                        {
                            handle_error("read");
                            break;
                        }
                    }
                    else if (n == 0)
                    {
                        // 删除sockfd
                        epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
                        close(sockfd);
                        events[i].data.fd = -1;
                    }
                    else
                    {
                        //设置用于写操作的文件描述符
                        ev.data.fd = sockfd;
                        //设置用于注测的写操作事件
                        ev.events = EPOLLOUT | EPOLLET;
                        //修改sockfd上要处理的事件为EPOLLOUT
                        epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                        printf("Client: %s\n", buf_read);
                    }
                }
                else if (events[i].events & EPOLLOUT) // 如果有数据发送
                {
                    sockfd = events[i].data.fd;
                    write(sockfd, buf_write, strlen(buf_write));
                    //设置用于读操作的文件描述符
                    ev.data.fd = sockfd;
                    //设置用于注测的读操作事件
                    ev.events = EPOLLIN | EPOLLET;
                    //修改sockfd上要处理的事件为EPOLIN
                    epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                }
            }
        }
    } while (0);

    if (server_fd != -1)
        close(server_fd);

    return 0;
}