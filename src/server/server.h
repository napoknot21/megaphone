#ifndef SERVER_H
#define SERVER_H

void * handle_connection (void *p_client_socket);
int check (int exp, const char *msg);

#endif
