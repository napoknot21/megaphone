#ifndef CLIENT_H
#define CLIENT_H

#include "megaphone.h"

int set_socket(int*, int, int, const char*, uint16_t);

int client_send_dataflow(const struct host*, const struct packet*, char**);
int client_send_datagram(int, const struct packet*);

#endif
