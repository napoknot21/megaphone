#include "core.h"
#include <string.h>
#include <arpa/inet.h>

struct sockaddr format_addr(int family, const char * ip, uint16_t port)
{
	struct sockaddr addr;
	memset(&addr, 0x0, sizeof(addr));

	addr.sa_family = family;
	port = htons(port);

	memmove(addr.sa_data, &port, 2);
	inet_pton(family, ip, addr.sa_data + 6);

	return addr;
}


