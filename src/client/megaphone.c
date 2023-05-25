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

struct packet * mp_signup(char * username, size_t len) 
{
	struct packet * p = make_packet();

	p->data = username;
	p->size = len;

	struct mp_header mhd = {SIGNUP, 0, 0, 0, p->size};
	forge_header(MP_CLIENT_SIDE, &p->header, mhd);

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
	p->size = strlen(filename);
	p->data = malloc(p->size);

	memmove(p->data, filename, p->size);

	struct mp_header mhd = {UPLOAD_FILE, se->uid, thread, 0, p->size};
	forge_header(MP_CLIENT_SIDE, &p->header, mhd);	

	return p;
}

struct packet * mp_download_file(const struct session * se, uint16_t thread, char * filename)
{
	struct packet * p = make_packet();
	p->size = strlen(filename);
	p->data = malloc(p->size);

	memmove(p->data, filename, p->size);

	struct mp_header mhd = {DOWNLOAD_FILE, se->uid, thread, MP_UDP_PORT, p->size};
	forge_header(MP_CLIENT_SIDE, &p->header, mhd);	

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
		p = mp_signup(argv[0], strlen(argv[0]));	
		break;
	
	case POST:
		if(argc <= 1)
		{
			printf("[!] Usage : post <thread> <message>\n");
			break;
		}

		sscanf(argv[0], "%hd", &thread);	
		struct post message_post = {MESSAGE, se->uid, argv[1]};

		p = mp_upload_post(se, &message_post, thread);
		break;	

	case FETCH:
		if(argc <= 1)
		{
			printf("[!] Usage : fetch <thread> <size>\n");
			return NULL;
		}

		sscanf(argv[0], "%hd", &thread);
		sscanf(argv[1], "%hd", &n);	

		p = mp_request_threads(se, thread, &n);
		break;

	case SUBSCRIBE:	
		sscanf(argv[0], "%hd", &thread);	
		p = mp_subscribe(se, thread);
		break;
	
	case UPLOAD_FILE: 
		if(argc <= 1)
		{
			printf("[!] Usage : upload <thread> <filename>\n");
			break;
		}

		sscanf(argv[0], "%hd", &thread);	
		p = mp_upload_file(se, thread, argv[1]);

		break;

	case DOWNLOAD_FILE:
		if(argc <= 1)
		{
			printf("[!] Usage : download <thread> <filename>\n");
			break;
		}

		sscanf(argv[0], "%hd", &thread);	
		p = mp_download_file(se, thread, argv[1]);

		break;

	default:
		printf("[-] This request code is unknown!\n");
		help();
	}	

	return p;
}

void mp_recv_posts(int fd, uint16_t n)
{
	struct packet rp;
	struct mp_post_header mph;	
	
	memset(&rp, 0x0, sizeof(rp));
	rp.header.fields = malloc(12 * FIELD_SIZE);	

	for(uint16_t k = 0; k < n; k++)
	{
		recv(fd, rp.header.fields, 12 * FIELD_SIZE, 0);
		melt_post_header(&mph, &rp.header);

		printf("Received data length : %d\n", rp.header.fields[11]);

		char * data = malloc(mph.len + 1);
		memset(data, 0x0, mph.len + 1);

		recv(fd, data, mph.len, 0);

		printf("[i][%d] %d -> %s\n", mph.nthread, mph.len, data);
	}
}

/*
 * This function manages the receive data
 * from the server, depending on the request
 * code.
 */

int mp_recv(const struct host * cl, struct session * se, const struct packet * context, struct packet * p)
{
	struct mp_header mhd;
	melt_header(MP_SERVER_SIDE, &mhd, &p->header);

	char * data = NULL;
	
	if(context->data)
	{
		data = malloc(context->size + 1);
		memset(data, 0x0, context->size + 1);
		memmove(data, context->data, context->size);
	}

	switch(mhd.rc)
	{
	case SIGNUP:
		se->uid = mhd.uuid;	
		printf("[i] You successfully signed-up to the network!\n");
		break;

	case POST:
		printf("[i] Your post has been successfully uploaded on thread %d!\n", mhd.nthread);
		break;

	case FETCH:
		printf("[i] %d post(s) fetched!\n", mhd.n);
		mp_recv_posts(cl->tcp_sock, mhd.n);
		break;

	case SUBSCRIBE:
		printf("[i] You successfully subscribed to thread %d!\n", mhd.nthread);
		break;

	case UPLOAD_FILE:	
		upload(AF_INET, DEFAULT_BOOTSTRAP, mhd.n, se, data);

		break;

	case DOWNLOAD_FILE:
		download(data, MP_UDP_PORT);
		break;

	default:
		printf("[-] Error %d has occured!\n", mhd.rc);
	}

	free(data);

	return 0;
}
