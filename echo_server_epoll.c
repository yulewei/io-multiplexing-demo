#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
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

    struct epoll_event event;
    struct epoll_event eventlist[MAX_EVENTS];
    int client_count = 0;

    int epfd = epoll_create1(EPOLL_CLOEXEC);

    // 把 server_socket 加到感兴趣的文件描述符列表中
    event.events = EPOLLIN;
    event.data.fd = server_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &event);

    printf("server started ...\n");
    while (1) {
        printf("call epoll_wait(), total client count: %d\n", client_count);
        int event_count = epoll_wait(epfd, eventlist, client_count + 1, -1);
        if (event_count <= 0) {
            fatal("epoll_wait failed");
            return 0;
        }

        for (int i = 0; i < event_count; i++) {
            int fd = (int) eventlist[i].data.fd;

            if (fd == server_socket) {
                int client_socket = accept(fd, NULL, 0);
                make_nonblocking(client_socket);
                printf("accepting new connection, client_fd %d\n", client_socket);

                // 把 client_socket 加到感兴趣的文件描述符列表中
                event.events = EPOLLIN;
                event.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);
                client_count++;
            } else {
                int client_socket = fd;
                // 读取，然后 echo
                long result = do_read_and_echo(client_socket);
                if (result > 0) {
                    printf("client_fd %d, reading and echoing %zd bytes\n", client_socket, result);
                } else {
                    printf("client_fd %d, closing\n", client_socket);
                    // close 后会自动从感兴趣的文件描述符列表中移除
                    close(client_socket);
                    client_count--;
                }
            }
        }
    }
}

