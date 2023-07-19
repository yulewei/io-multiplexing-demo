#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "common.h"

volatile sig_atomic_t client_count = 0;

void handler() {
    client_count--;
}

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

    signal(SIGCHLD, handler);

    printf("server started ...\n");
    while (1) {
        int client_socket = accept(server_socket, NULL, 0);
        client_count++;
        printf("accepting new connection, client_fd %d, total client count: %d\n", client_socket, client_count);
        int pid;
        if ((pid = fork()) == 0) {
            // 读取，然后 echo
            char buf[BUF_SIZE];
            while (1) {
                ssize_t result = read(client_socket, buf, sizeof(buf));
                if (result > 0) {
                    printf("client_fd %d, reading and echoing %zd bytes\n", client_socket, result);
                    write(client_socket, "> ", 2);
                    write(client_socket, buf, result);
                } else {
                    printf("client_fd %d, closing\n", client_socket);
                    close(client_socket);
                    break;
                }
            }
            // 退出子进程
            exit(0);
        }
    }
}
