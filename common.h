#define SERVER_PORT 8088
#define BUF_SIZE 4096
#define MAX_SOCKET 1024
#define MAX_EVENTS 16

void fatal(char *string);
void make_nonblocking(int fd);
long do_read_and_echo(int client_socket);

typedef struct node_t {
    int val;
    struct node_t *next;
} node_t;

typedef struct list_t {
    struct node_t *head;
    struct node_t *tail;
} list_t;

list_t *list_create();
void list_insert(list_t *list, int item);
void list_remove(list_t *list, int item);
node_t *list_find(list_t *list, int item);
int list_size(list_t *list);
void list_print(list_t *list);