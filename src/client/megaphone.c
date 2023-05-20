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

struct packet * mp_signup(char * username) 
{
	struct packet * p = make_packet();

	p->header.fields = malloc(FIELD_SIZE);
	p->header.size = 1;

	uint16_t lfield = fusion(SIGNUP, 0);
		
	p->header.fields[MP_FIELD_CR_UUID] = htons(lfield);
	p->data = username;
	p->size = strlen(username);

	printf("[i] username: %s\n", username);
	return p;
}

/*
 * This method builds the packet for
 * the corresponding request code POST.
 */

struct packet * mp_upload_post(const struct session * se, struct post * pt, uint16_t thread) 
{
	struct packet * p = make_packet(); 

	p->data = pt->data;
	p->size = strlen(pt->data);

	struct mp_header mhd = {POST, se->uid, thread, 0, p->size};

	fill_header(&p->header, mhd);

	return p;
}

/*
 * This method builds the packet for
 * the corresponding request code FETCH.
 */

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t n) 
{
	struct packet * p = make_packet();
	struct mp_header mhd = {FETCH, se->uid, thread, n, 0};

	fill_header(&p->header, mhd);

	return p;
}

/*
 * This methode builds packet for
 * the corresponding request code
 * SUBSCRIBE.
 */

struct packet * mp_subscribe(const struct session * se, uint16_t thread) 
{
	struct packet * p = malloc(sizeof(struct packet));
	struct mp_header mhd = {SUBSCRIBE, se->uid, thread, 0, 0};
	
	fill_header(&p->header, mhd);

	return p;
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
		break;
	
	case POST:
		if(argc <= 1)
		{
			printf("[!] Usage : post <thread> <message>");
		}

		memmove(&thread, argv[0], 2);
		struct post message_post = {MESSAGE, se->uid, argv[1]};

		p = mp_upload_post(se, &message_post, thread);
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

int mp_recv(struct session * se, const char * data)
{
	struct packet * p = melt_tcp_packet(data);
	struct mp_header mhd;

	melt_header(&mhd, &p->header);

	switch(mhd.rc)
	{
	case SIGNUP:
		se->uid = mhd.uuid;	
		printf("[i] You successfully signed-up to the network!\n");
		break;

	case POST:
		printf("[i] Your post has been successfully uploaded!\n");
		break;

	case FETCH:
		printf("[i] %d post(s) fetched!\n%s\n", mhd.n, p->data);
		break;

	case SUBSCRIBE:
		printf("[i] You successfully subscribed to %d thread!\n", mhd.nthread);
		break;

	case DOWNLOAD:
		break;

	default:
		printf("[-] Error %d has occured!\n", mhd.rc);
	}

	return 0;
}
