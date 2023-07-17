#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <event2/util.h>
#include <event2/event.h>
#include <unistd.h>
#include "common.h"

struct server_state {
    struct event_base *base;
    struct event *event;
    int client_count;
};

struct client_state {
    struct event_base *base;
    struct event *event;
    struct server_state *server_state;
};

void do_read(evutil_socket_t client_socket, short events, void *arg) {
    struct client_state *state = arg;
    // 读取，然后 echo
    long result = do_read_and_echo(client_socket);
    if (result > 0) {
        printf("client_fd %d, reading and echoing %zd bytes\n", client_socket, result);
    } else {
        printf("client_fd %d, closing\n", client_socket);
        state->server_state->client_count--;
        close(client_socket);
        event_free(state->event);
        free(state);
    }
}

void do_accept(evutil_socket_t server_socket, short event, void *arg) {
    struct server_state *state = arg;
    int client_socket = accept(server_socket, NULL, 0);
    state->client_count++;

    evutil_make_socket_nonblocking(client_socket);
    printf("accepting new connection, client_fd %d, total client count: %d\n", client_socket, state->client_count);

    struct client_state *client_state = malloc(sizeof(struct client_state));
    struct event *client_event = event_new(state->base, client_socket, EV_READ | EV_PERSIST, do_read,
                                           (void *) client_state);
    client_state->base = state->base;
    client_state->event = client_event;
    client_state->server_state = state;

    event_add(client_event, NULL);
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
    evutil_make_socket_nonblocking(server_socket);

    struct event_base *base = event_base_new();

    struct server_state *state = malloc(sizeof(struct server_state));
    struct event *listener_event = event_new(base, server_socket, EV_READ | EV_PERSIST, do_accept, (void *) state);
    state->base = base;
    state->event = listener_event;
    state->client_count = 0;

    event_add(listener_event, NULL);

    printf("server started ...\n");

    // 运行 event loop
    event_base_dispatch(base);
}
