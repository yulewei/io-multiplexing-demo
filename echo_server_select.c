#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "common.h"

int main(int argc, char *argv[]) {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) fatal("socket failed");

    int ret = bind(server_socket, (struct sockaddr *) &sa, sizeof(sa));
    if (ret < 0) fatal("bind failed");

    ret = listen(server_socket, 512);
    if (ret < 0) fatal("listen failed");

    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    make_nonblocking(server_socket);

    fd_set readfds;
    int client_count;
    list_t *client_socket_list = list_create();

    printf("server started ...\n");
    while (1) {
        // 必须在每次调用前初始化感兴趣的文件描述符列表
        FD_ZERO(&readfds);
        // 把 server_socket 加到感兴趣的文件描述符列表中
        FD_SET(server_socket, &readfds);

        // 把 client_socket_list 加到感兴趣的文件描述符列表中
        int maxfd = server_socket;
        client_count = 0;
        node_t *node;
        for (node = client_socket_list->head; node != NULL; node = node->next) {
            FD_SET(node->val, &readfds);
            if (node->val > maxfd) {
                maxfd = node->val;
            }
            client_count++;
        }

        printf("call select(), total client count: %d\n", client_count);
        ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ret <= 0) {
            fatal("select failed");
            return 0;
        }

        if (FD_ISSET(server_socket, &readfds)) {
            int client_socket = accept(server_socket, NULL, 0);
            make_nonblocking(client_socket);
            printf("accepting new connection, client_fd %d\n", client_socket);
            // 把 client_socket 加到感兴趣的文件描述符列表中
            list_insert(client_socket_list, client_socket);
        }

        for (node = client_socket_list->head; node != NULL; node = node->next) {
            int client_socket = node->val;
            if (!FD_ISSET(client_socket, &readfds)) {
                continue;
            }
            // 读取，然后 echo
            long result = do_read_and_echo(client_socket);
            if (result > 0) {
                printf("client_fd %d, reading and echoing %zd bytes\n", client_socket, result);
            } else {
                printf("client_fd %d, closing\n", client_socket);
                close(client_socket);
                // 把 client_socket 移除感兴趣的文件描述符列表中
                list_remove(client_socket_list, client_socket);
            }
        }
    }
}

