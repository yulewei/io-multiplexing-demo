#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
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

    struct pollfd pollfds[MAX_SOCKET];
    // 把 server_socket 加到感兴趣的文件描述符列表中
    pollfds[0].fd = server_socket;
    pollfds[0].events = POLLIN; // 感兴趣的事件
    list_t *client_socket_list = list_create();

    printf("server started ...\n");
    while (1) {
        // 把 client_socket_list 加到感兴趣的文件描述符列表中
        int i = 1, client_count = 0;
        node_t *node;
        for (node = client_socket_list->head; node != NULL; node = node->next, i++) {
            pollfds[i].fd = node->val;
            pollfds[i].events = POLLIN; // 感兴趣的事件
            client_count++;
        }
        printf("call poll(), total client count: %d\n", client_count);
        ret = poll(pollfds, client_count + 1, -1);
        if (ret <= 0) {
            fatal("poll failed");
            return 0;
        }

        for (i = 0; i < client_count + 1; i++) {
            struct pollfd pfd = pollfds[i];

            if (!(pfd.revents & POLLIN))
                continue;

            printf("fd=%d; events: %s%s%s\n", pfd.fd,
                   (pfd.revents & POLLIN) ? "POLLIN " : "",
                   (pfd.revents & POLLHUP) ? "POLLHUP " : "",
                   (pfd.revents & POLLERR) ? "POLLERR " : "");

            if (pfd.fd == server_socket) {
                int client_socket = accept(pfd.fd, NULL, 0);
                make_nonblocking(client_socket);
                printf("accepting new connection, client_fd %d\n", client_socket);
                // 把 client_socket 加到感兴趣的文件描述符列表中
                list_insert(client_socket_list, client_socket);
            } else {
                int client_socket = pfd.fd;
                // 读取，然后 echo
                long result = do_read_and_echo(client_socket);
                if (result > 0) {
                    printf("client_fd %d, reading and echoing %zd bytes\n", client_socket, result);
                } else {
                    printf("client_fd %d, closing\n", client_socket);
                    close(client_socket);
                    // 把 client_socket 移除感兴趣的文件描述符列表中
                    list_remove(client_socket_list, client_socket);
                    pollfds[i].fd = -1;
                }
            }
        }
    }
}

