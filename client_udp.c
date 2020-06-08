#include "utils.h"


int main()
{
    int server_fd = -1;
    do
    {
        struct sockaddr_in server_addr;
        memset((void *)&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_family = AF_INET;
        char buf_read[WRITE_MAX_SIZE];
        memset(buf_read, 0, sizeof(buf_read));

        server_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_fd == -1)
        {
            handle_error("socket");
            break;
        }
        if (sendto(server_fd, SEND_2_CLIENT_MSG, 
                    strlen(SEND_2_CLIENT_MSG), 
                    0, 
                    (SA *)&server_addr, 
                    sizeof(server_addr)) != strlen(SEND_2_CLIENT_MSG))
        {
            handle_error("sendto");
            break;
        }
        socklen_t server_len = sizeof(server_len);
        ssize_t recv_size = recvfrom(server_fd, buf_read, READ_MAX_SIZE, 0, (SA *)&server_addr, &server_len);

        printf("recv data: %s size: %ld\n", buf_read, recv_size);
    } while (0);

    if (server_fd != -1)
        close(server_fd);
    return 0;
}