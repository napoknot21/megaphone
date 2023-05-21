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

	forge_header(MP_CLIENT_SIDE, &p->header, mhd);

	return p;
}

/*
 * This method builds the packet for
 * the corresponding request code FETCH.
 */

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t * n) 
{
	struct packet * p = make_packet();
	struct mp_header mhd = {FETCH, se->uid, thread, *n, 0};

	forge_header(MP_CLIENT_SIDE, &p->header, mhd);

	return p;
}

/*
 * This methode builds packet for
 * the corresponding request code
 * SUBSCRIBE.
 */

struct packet * mp_subscribe(const struct session * se, uint16_t thread) 
{
	struct packet * p = make_packet();
	struct mp_header mhd = {SUBSCRIBE, se->uid, thread, 0, 0};
	
	forge_header(MP_CLIENT_SIDE, &p->header, mhd);

	return p;
}

struct packet * mp_upload_file(const struct session * se, uint16_t thread, char * filename)
{
	struct packet * p = make_packet();
	struct mp_header mhd = {UPLOAD_FILE, se->uid, thread, 0, strlen(filename)};

	forge_header(MP_CLIENT_SIDE, &p->header, mhd);
	p->data = filename;

	return p;
}

struct packet * mp_download_file(const struct session * se, uint16_t thread, char * filename)
{
	struct packet * p = make_packet();
	struct mp_header mhd = {DOWNLOAD_FILE, se->uid, thread, MP_UDP_PORT, strlen(filename)};

	forge_header(MP_CLIENT_SIDE, &p->header, mhd);
	p->data = filename;

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
			break;
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

		memmove(&thread, argv[0], 2);
		memmove(&n, argv[1], 2);

		p = mp_request_threads(se, thread, &n);
		break;

	case SUBSCRIBE:	
		memmove(&thread, argv[0], 2);
		p = mp_subscribe(se, thread);
		break;
	
	case UPLOAD_FILE: 
		if(argc <= 1)
		{
			printf("[!] Usage : upload <thread> <filename>\n");
			break;
		}

		memmove(&thread, argv[0], 2);
		p = mp_upload_file(se, thread, argv[1]);

		break;

	case DOWNLOAD_FILE:
		if(argc <= 1)
		{
			printf("[!] Usage : download <thread> <filename>\n");
			break;
		}

		memmove(&thread, argv[0], 2);
		p = mp_download_file(se, thread, argv[1]);

		break;

	default:
		printf("[-] This request code is unknown!\n");
	}	

	return p;
}

void mp_recv_posts(int fd, uint16_t n)
{
	uint16_t thread;
	char origin[10];
	char pseudo[10];

	char len;	

	for(uint16_t k = 0; k < n; k++)
	{
		recv(fd, &thread, 2, 0);
		recv(fd, origin, 10, 0);
		recv(fd, pseudo, 10, 0);
		recv(fd, &len, 1, 0);

		char * data = malloc((size_t) len);

		recv(fd, data, (size_t) len, 0);
		printf("[i][%d] %s -> %s\n%s\n", thread, origin, pseudo, data);
	}
}

/*
 * This function manages the receive data
 * from the server, depending on the request
 * code.
 */

int mp_recv(const struct host * cl, struct session * se, const char * data)
{
	struct packet * p = melt_tcp_packet(data);
	struct mp_header mhd;

	melt_header(MP_SERVER_SIDE, &mhd, &p->header);

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
		printf("[i] %d post(s) fetched!\n", mhd.n);
		mp_recv_posts(cl->tcp_sock, mhd.n);
		break;

	case SUBSCRIBE:
		printf("[i] You successfully subscribed to %d thread!\n", mhd.nthread);
		break;

	case UPLOAD:
		upload();
		break;

	case DOWNLOAD:
	//	download();
		break;

	default:
		printf("[-] Error %d has occured!\n", mhd.rc);
	}

	return 0;
}
