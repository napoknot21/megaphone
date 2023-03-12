#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

node_t *head = NULL;
node_t *tail = NULL;

void enqueue (int *client_socket) 
{
    node_t *new_node = malloc (sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->next = NULL;
    if (tail == NULL) {
        head = new_node;
    } else {
        tail->next = new_node;
    }
    tail = new_node;
}

int *dequeue () 
{
    if (head == NULL) {
        return NULL;
    }
    int *result = head->client_socket;
    node_t *tmp = head;
    head = head->next;
    if (head == NULL) tail = NULL;
    free(tmp);
    return result
}


void freeQueue ()
{
    node_t *tmp;

    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp)
    }
}
