#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>

void fatal(char *string) {
    perror(string);
    exit(1);
}

void make_nonblocking(int fd) {
    int flags;
    fcntl(fd, F_GETFL, &flags);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * 接收到客户端的字符串输入，然后响应相同的字符串
 */
long do_read_and_echo(int client_socket) {
    char buf[BUF_SIZE];
    long total = 0;
    while (1) {
        ssize_t result = read(client_socket, buf, sizeof(buf));
        if (result <= 0) break;

        write(client_socket, "> ", 2);
        write(client_socket, buf, result);
        total += result;
    }
    return total;
}

/**
 * 创建链表
 */
list_t *list_create() {
    list_t *list = (list_t *) malloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

/**
 * 在链表末尾插入新项
 */
void list_insert(list_t *list, int item) {
    node_t *node = (node_t *) malloc(sizeof(node_t));
    node->val = item;
    node->next = NULL;

    // 插入首个链表项
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }

    node_t *tail = list->tail;
    tail->next = node;
    list->tail = node;
}

/**
 * 查找链表项
 */
node_t *list_find(list_t *list, int item) {
    node_t *node = list->head;
    while (node != NULL) {
        if (node->val == item) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/**
 * 删除链表项
 */
void list_remove(list_t *list, int item) {
    node_t *prev = NULL;
    node_t *node = list->head;
    while (node != NULL) {
        if (node->val == item) {
            // 删除第一个链表项
            if (prev != NULL) {
                prev->next = node->next;
            } else {
                list->head = node->next;
            }
            // 删除最后一个链表项
            if (node == list->tail) {
                list->tail = prev;
            }
            free(node);
            break;
        }
        prev = node;
        node = node->next;
    }
}

int list_size(list_t *list) {
    int size = 0;
    node_t *node = list->head;
    while (node != NULL) {
        size++;
        node = node->next;
    }
    return size;
}

void list_print(list_t *list) {
    node_t *node = list->head;
    fputs("[ ", stdout);
    while (node != NULL) {
        printf("%d ", node->val);
        node = node->next;
    }
    fputs("]\n", stdout);
    fflush(stdout);
}