#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"

struct node *head = NULL;
struct node *current = NULL;

int isEmpty () 
{
    return head == NULL;
}


struct node * createNode (int *client_socket) 
{
    struct node *new_node = (struct node *) malloc (sizeof(struct node));
    new_node->client_socket = client_socket;
    new_node->next = NULL;
    return new_node;
}


struct node * createQueue (int *client_socket)
{
    head = createNode(client_socket);
    return head;
}


void enqueue (int *client_socket) 
{
    struct node *new_node = (struct node *) malloc (sizeof(struct node));

    new_node->client_socket = client_socket;
    new_node->next = head;
    
    head = new_node;
}


void deleteNode (struct node *to_delete)
{
    if (to_delete == NULL) return;

    if (to_delete == head) {
        if (to_delete->next == NULL) {
            free(head);
        } else {
            current = head;
            head = head->next;
            free(current);
        }
        return;
    } 
    
    current = head;
    while (current->next != to_delete && current->next != NULL) {
        current = current->next;
    }
    
    if (current->next == NULL) {
        return;
    }

    currrent->next = to_delete->next;
    free(to_delete);
}


void dequeue () 
{
    deleteNode(head);
}


void freeNode (struct node *to_delete)
{
    free(to_delete->client_socket);
    free(to_delete);
}


void freeQueue ()
{
}
