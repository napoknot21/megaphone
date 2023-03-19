#ifndef QUEUE_H
#define QUEUE_H

struct node {
    int *client_socket;
    struct node *next;
};


/**
 * @brief : Check and verify if the linked list (queue) is empty or not
 * @return 1 or 0
 */
int isEmpty();


/**
 * @brief : Create a node structre from a pointer to a client_socket
 * @param client_socket : pointer to client_socket
 */
struct node * createNode (int *client_socket);


/**
 * @brief : Create a queue from client_socket pointer 
 * @param client_socket : the pointer to the client socket
 */
struct node * createQueue (int *client_socket);


/**
 * @brief : Add to the head a new socket
 * @param client_socket : socket for the client
 */
void enqueue (int *client_socket);


/**
 * @brief : Delete a node from the queue
 * @param to_delete : pointer to the node
 */
void deleteNode (struct node *to_delete);


/**
 * @brief : Delete a a node and return its client socket
 * @return the socket of the tail
 */
void dequeue (void);


/**
 * 
 */
void freeQueue (void);


/**
 * @brief : Print the queue
 */
void printQueue ();

#endif
