#include "utils.h"

#define SERVER_HOST "127.0.0.1"

int main()
{
    int server_fd = -1;
    do
    {
        struct sockaddr_in server_addr;
        memset((void*)&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_family = AF_INET;
        char buf_write[READ_MAX_SIZE] = SEND_2_SERVER_MSG;
        char buf_read[WRITE_MAX_SIZE];
        memset(buf_read, 0, sizeof(buf_read));
        
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            handle_error("socket");
            break;
        }
        if (inet_aton(SERVER_HOST, (struct in_addr*)&server_addr.sin_addr) == 0)
        {
            handle_error("inet_aton");
            break;
        }
        if (connect(server_fd, (const SA*)&server_addr, sizeof(struct sockaddr_in)) == -1)
        {
            handle_error("client connect to server");
            break;
        }
        printf("Connect successfully...\n");
        
        ssize_t write_size = write(server_fd, buf_write, strlen(buf_write));
        if (write_size == -1)
        {
            handle_error("write");
            break;
        }
        ssize_t recv_size = read(server_fd, buf_read, sizeof(buf_read));
        if (recv_size == -1)
        {
            handle_error("read");
            break;
        }
        printf("recv data: %s size: %ld\n", buf_read, recv_size);
    } while (0);

    if (server_fd != -1)
        close(server_fd);
    return 0;
}