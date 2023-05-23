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

char * forge_tcp_packet(const struct packet * p, size_t * len)
{
    size_t hd_size = p->header.size * FIELD_SIZE;

    printf("[i][forge]\n\tHeader size \t-- %ld\n\tData size \t-- %ld\n", hd_size, p->size);

    *len = p->size + hd_size;

    char * buffer = malloc(*len);
    memset(buffer, 0x0, *len); 

    memmove(buffer, p->header.fields, hd_size);
    if(p->size) memmove(buffer + hd_size, p->data, p->size); 

    return buffer;
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
	printf("[i] Looking for a fitting network interface ...\n");

	if(getifaddrs(&ifa) == -1)
	{
		printf("[-] There's no network devices!\n");
		return 0;
	}

	int family;
	unsigned index = 0;	

	for(struct ifaddrs * i = ifa; i != NULL; i = i->ifa_next)
	{
		family = i->ifa_addr->sa_family;
		int fl = i->ifa_flags & flags;

		const char * if_name = i->ifa_name;

		if(family != domain || fl != flags || !strcmp(if_name, "lo"))
		{
			continue;
		}

		index = if_nametoindex(if_name);
		if(index) 
		{
			printf("[i] \"%s\" interface seems to fit!\n", i->ifa_name);
			break;
		}
	}

	freeifaddrs(ifa);

	if(!index)
	{
		printf("[-] No corresponding network interface found!\n");
	}

	return index;
}
