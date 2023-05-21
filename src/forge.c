#include "forge.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>

const char * bufferize(const struct header * hd)
{
	char * buffer = malloc(hd->size);

	for(size_t k = 0; k < hd->size; k++)
	{
		memmove(buffer + (k + 1) * FIELD_SIZE, hd->fields + k, FIELD_SIZE);
	}

	return (const char*) buffer;
}

const char * forge_tcp_packet(const struct packet * p)
{
    size_t data_size = p->size;
    data_size = !(data_size & 1) ? data_size : data_size + 1;

    size_t hd_size = p->header.size;

    printf("[i][forge]\n\tHeader size \t-- %ld\n\tData size \t-- %ld\n", hd_size, data_size);

    size_t bs = data_size + p->header.size + 1;

    char * buffer = malloc(bs);
    memset(buffer, 0x0, bs);
    
    const char * header_bytes = bufferize(&p->header);

    memmove(buffer, header_bytes, hd_size);
    memmove(buffer + hd_size, p->data, data_size);

    return (const char*) buffer;
}

struct packet * make_packet()
{
	struct packet * p = malloc(sizeof(struct packet));

	memset(p, 0x0, sizeof(struct packet));
	memset(&p->header, 0x0, sizeof(struct header));

	return p;
}

void free_packet(struct packet * p)
{
	free(p->header.fields);
	free(p->data);
	free(p);
}

unsigned get_interface(int domain, int flags)
{
	struct ifaddrs * ifa;

	if(getifaddrs(&ifa) == -1)
	{
		printf("[-] There's no network devices!\n");
		return 0;
	}

	int family;

	for(struct ifaddrs * i = ifa; i != NULL; i = ifa->ifa_next)
	{
		family = i->ifa_addr->sa_family;
		int fl = i->ifa_flags & flags;

		const char * if_name = i->ifa_name;

		if(family != domain || fl != flags || !strcmp(if_name, "lo"))
		{
			continue;
		}

		unsigned index = if_nametoindex(if_name);
		if(index) 
		{
			printf("[i] Network interface found!\n");
			return index;
		}
	}

	printf("[-] No corresponding network interface found!\n");

	return 0;
}
