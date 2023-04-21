#include "megaphone.h"
#include "../forge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

/*
 * This method builds the packet for
 * the corresponding request code
 * SIGNUP.
 */

struct packet * mp_signup(const char * username) 
{
	struct packet * p = malloc(sizeof(struct packet));
	
	memset(p, 0x0, sizeof(struct packet));
	memset(&p->header, 0x0, sizeof(p->header));

	p->header.code = SIGNUP;
	p->data = username;
	p->size = strlen(username);

	printf("[i] username: %s\n", username);
	return p;
}

/*
 * This method builds the packet for
 * the corresponding request code POST.
 */

struct packet * mp_upload_post(const struct session * se, const struct post * p) {
	struct packet * pa = malloc(sizeof(struct packet));

	memset(pa, 0x0, sizeof(struct packet));
	memset(&pa->header, 0x0, sizeof(pa->header));
	memset(&pa->data, 0x0, sizeof(pa->data));

	pa->data = p->data;
	pa->size = strlen(p->data);
	pa->header.code = POST;
	pa->header.id = htonl(se->uid);
	pa->header.fields = malloc(sizeof(3*sizeof(uint16_t)));
	pa->header.fields[0] = htonl(p->thread);
	pa->header.fields[1] = htonl(0);
	pa->header.fields[2] = strlen(p->data);

	return pa;
}

/*
 * This method builds the packet for
 * the corresponding request code FETCH.
 */

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t n) {
	struct packet * pa = malloc(sizeof(struct packet));

	pa->header.fields = malloc(sizeof(3*sizeof(uint16_t)));

	memset(pa, 0x0, sizeof(struct packet));
	memset(&pa->header, 0x0, sizeof(pa->header));
	memset(&pa->data, 0x0, sizeof(pa->data));
	memset(&pa->header.fields, 0x0, sizeof(pa->header.fields));

	pa->size = htonl(0);
	pa->header.code = FETCH;
	pa->header.id = htonl(se->uid);
	pa->header.fields[0] = htonl(thread);
	pa->header.fields[1] = htonl(n);
	
	return pa;
}

/*
 * This methode builds packet for
 * the corresponding request code
 * SUBSCRIBE.
 */

struct packet * mp_subscribe(const struct session * se, uint16_t thread) {
	struct packet * pa = malloc(sizeof(struct packet));

	pa->header.fields = malloc(sizeof(3*sizeof(uint16_t)));

	memset(pa, 0x0, sizeof(struct packet));
	memset(&pa->header, 0x0, sizeof(pa->header));
	memset(&pa->data, 0x0, sizeof(pa->data));
	memset(&pa->header.fields, 0x0, sizeof(pa->header.fields));

	pa->size = 0;
	pa->header.code = SUBSCRIBE;
	pa->header.id = htonl(se->uid);
	pa->header.fields[0] = htonl(thread);
	pa->header.fields[1] = htonl(0);
	
	return pa;
}

/*
 * This method refers the correct packet
 * builder, depending on the request
 * code. It returns a packet structure.
 */

struct packet * mp_request_for(const struct session * se, const request_code_t rc, size_t argc, char ** argv) 
{
	struct packet * p = NULL;
	uint16_t thread, n;

	switch(rc)
	{
	case SIGNUP:
		printf("[!] Signing-up with username %s\n", argv[0]);
		p = mp_signup(argv[0]);
		// printf("%s", p->data);
		break;
	
	case POST:
		if(argc <= 1)
		{
			printf("[!] Usage : post <thread> <message>");
		}

		memmove(&thread, argv[0], 2);
		struct post message_post = {MESSAGE, thread, argv[1]};

		p = mp_upload_post(se, &message_post);
		break;
	
	case FETCH:
		if(argc <= 1)
		{
			printf("[!] Usage : fetch <thread> <size>");
			return NULL;
		}

		memmove(&thread, &argv[0], 2);
		memmove(&n, argv[1], 2);

		p = mp_request_threads(se, thread, n);
		break;

	case SUBSCRIBE:	
		memmove(&thread, &argv[0], 2);
		p = mp_subscribe(se, thread);
		break;
	
	case DOWNLOAD:
		break;

	default:
		printf("[-] This request code is unknown!\n");
	}	

	return p;
}

/*
 * This function manages the receive data
 * from the server, depending on the request
 * code.
 */

int mp_recv(struct session * se, const request_code_t rc, const char * data)
{
	switch(rc)
	{
	case SIGNUP:
		memmove(&se->uid, data, sizeof(uuid_t));
		printf("[+] You successfully signed-up to the network!\n");
		break;
	case POST:
		break;
	case FETCH:
		break;
	case SUBSCRIBE:
		break;
	case DOWNLOAD:
		break;
	case UNKNOWN:
		break;
	}

	return 0;
}
