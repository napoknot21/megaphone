#include "megaphone.h"
#include "../forge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct packet * mp_signup(const char * username) 
{
	struct packet * p = malloc(sizeof(struct packet));
	
	memset(p, 0x0, sizeof(struct packet));
	memset(&p->header, 0x0, sizeof(p->header));

	p->header.code = SIGNUP;

	return p;
}

struct packet * mp_upload_post(const struct session * se, const post_t post) {}

struct packet * mp_request_threads(const struct session * se, unsigned thread, unsigned n) {}

struct packet * mp_subscribe(const struct session * se, unsigned thread) {}

struct packet * mp_request_for(const struct session * se, const request_code_t rc, size_t argc, char ** argv) 
{
	struct packet * p = NULL;

	switch(rc)
	{
	case SIGNUP:
		if(!argc)
		{
			printf("Usage: signup <username>");
			return NULL;
		}

		printf("[!] Signing-up with username %s\n", argv[0]);
		p = mp_signup(argv[0]);	
		break;
	
	case POST:
		break;
	case FETCH:
		break;
	case SUBSCRIBE:
		break;
	case DOWNLOAD:
		break;

	default:
		printf("[-] This request code is unknown!\n");
	}	

	return p;
}

int mp_recv(struct session * se, const request_code_t rc, const char * argv)
{
	return 0;
}
