#include "protocol.h"
#include "core.h"
#include "utils/vector.h"
#include "utils/string.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

struct host * make_host()
{
	struct host * h = malloc(sizeof(struct host));
	memset(h, 0x0, sizeof(struct host));

	h->udp_socks = make_vector(NULL, NULL, sizeof(int));

	return h;
}

void free_host(struct host * h)
{
	free_vector(h->udp_socks);
	free(h);
}

void forge_header(int side, struct header * hd, const struct mp_header mhd)
{
	memset(hd, 0x0, sizeof(struct header));

	size_t fs = !side ? MP_HEADER_FIELD_SIZE : 3;

	hd->fields = malloc(fs * FIELD_SIZE);
	hd->size = fs;

	uint16_t cu = fusion(mhd.rc, mhd.uuid);	

	hd->fields[MP_FIELD_CR_UUID] = htons(cu);
	hd->fields[MP_FIELD_THREAD] = htons(mhd.nthread);
	hd->fields[MP_FIELD_NUMBER] = htons(mhd.n);
	
	if(side == MP_CLIENT_SIDE)
	{
		hd->fields[MP_FIELD_DATALEN] = htons(mhd.len);	
	}
}

void melt_header(int side, struct mp_header * mhd, const struct header * hd)
{
	memset(mhd, 0x0, sizeof(struct mp_header));

	uint16_t lfield = ntohs(hd->fields[MP_FIELD_CR_UUID]);

	mhd->rc = get_rq_code(lfield);
	mhd->uuid = get_uuid(lfield);

	mhd->nthread = ntohs(hd->fields[MP_FIELD_THREAD]);
	mhd->n = ntohs(hd->fields[MP_FIELD_NUMBER]);

	if(side == MP_CLIENT_SIDE)
	{
		mhd->len = ntohs(hd->fields[MP_FIELD_DATALEN]);
	}
}

void forge_post_header(struct header * hd, const struct mp_post_header mph)
{
	memset(hd, 0x0, sizeof(struct header));

	hd->size = 12;
	hd->fields = malloc(hd->size * FIELD_SIZE);

	hd->fields[0] = htons(mph.nthread);
	memmove(hd->fields + 1, mph.origin, 10);
	memmove(hd->fields + 6, mph.pseudo, 10);
	hd->fields[11] = htons(mph.len);	
}

void melt_post_header(struct mp_post_header * mph, const struct header * hd)
{
	memset(mph, 0x0, sizeof(struct mp_post_header));

	mph->nthread = ntohs(hd->fields[0]);
	memmove(mph->origin, hd->fields + 1, 10);
	memmove(mph->pseudo, hd->fields + 6, 10);
	mph->len = ntohs(hd->fields[11]);
}

uint16_t get_rq_code(uint16_t field)
{
	return (field & REQUEST_CODE_MASK) >> USER_ID_SIZE_BITS;
}

uid_t get_uuid(uint16_t field)
{
	return field & USER_ID_MASK;
}

request_code_t string_to_code(const char * str)
{
	if(!strcmp(str, "signup")) 
		return SIGNUP;
	else if(!strcmp(str, "post")) 
		return POST;
	else if(!strcmp(str, "fetch"))
		return FETCH;
	else if(!strcmp(str, "subscribe"))
		return SUBSCRIBE;
	else if(!strcmp(str, "upload"))
		return UPLOAD_FILE;
	else if(!strcmp(str, "download"))
		return DOWNLOAD_FILE;	

	return 0;
}

uint16_t fusion(uint16_t u, uint16_t v)
{
    uint16_t field;

    field = u << USER_ID_SIZE_BITS & REQUEST_CODE_MASK;
    field = field | (v & USER_ID_MASK);

    return field;
}

struct packet * melt_tcp_packet(const char * bytes)
{
    struct packet * p = make_packet(); 
    
    uint16_t lfield;
    memmove(&lfield, bytes, 2); 

    lfield = ntohs(lfield);
    uint16_t code = get_rq_code(lfield);

    if(code > DOWNLOAD_FILE)
    {
        free_packet(p);
        printf("[packet] received data is corrupted!\n");
        
        return NULL;
    }
    else if(code != SIGNUP)
    {
        p->header.size = MP_HEADER_FIELD_SIZE * FIELD_SIZE;
        p->header.fields = malloc(p->header.size);
        memmove(&p->header.fields, bytes, p->header.size);

	for(size_t i = 0; i < MP_HEADER_FIELD_SIZE; i++) p->header.fields[i] = ntohs(p->header.fields[i]);
    }

    return p;
}

struct post * copy_post(const struct post * pt)
{
	struct post * copy = malloc(sizeof(struct post));
	memset(copy, 0x0, sizeof(struct post));

	copy->type = pt->type;
	copy->uuid = pt->uuid;	

	copy->data = malloc(pt->len);
	memmove(copy->data, pt->data, pt->len);

	copy->len = pt->len;
	printf("Copying post ... %ld\n", copy->len);

	return copy;
}

void free_post(struct post * pt)
{
	if(pt->data) free(pt->data);
	free(pt);
}

struct thread * copy_thread(const struct thread * th)
{
	struct thread * copy = malloc(sizeof(struct thread));
	memset(copy, 0x0, sizeof(struct thread));

	copy->seed = th->seed;
	copy->addr = th->addr;

	copy->posts = copy_vector(th->posts);
	copy->files = copy_vector(th->files);

	return copy;
}

void free_thread(struct thread * th)
{
	free_vector(th->posts);
	free_vector(th->files);
	free(th);
}

struct session * copy_session(struct session * model)
{
	struct session * copy = malloc(sizeof(struct session));
	memset(copy, 0x0, sizeof(struct session));

	copy->uid = model->uid;

	size_t us = strlen(model->username);
	copy->username = malloc(us + 1);
	
	copy->username[us] = 0x0;
	memmove(copy->username, model->username, us + 1);

	return copy;
}

void free_session(struct session * se)
{
	free(se->username);
	free(se);
}

void print_post(const struct post * pt)
{	
	printf("From: %d\nContent: %s\n", pt->uuid, pt->data);	
}

void help()
{
	printf("Possible command : \n\n");
	printf("	- Use : signup pseudo, to resgister you as pseudo \n");
	printf("	- Use : post f message, to post a message on the thread f\n");
	printf("	- Use : fetch f n, to get n first message of thread f: \n");
	printf("	- Use : subscribe f, to subscribe to the thread f: \n");
	printf("	- Use : upload f file, to upload file on the thread f: \n");
	printf("	- Use : download f file, to download file from the thread f: \n");

}

void forge_udp_header(struct header * hd, const struct mp_udp_header muh)
{
	memset(hd, 0x0, sizeof(struct header));

	hd->size = 2;
	hd->fields = malloc(hd->size * FIELD_SIZE);	

	hd->fields[0] = htons(fusion(muh.rc, muh.uuid));
	hd->fields[1] = htons(muh.n);
}

void melt_udp_header(struct mp_udp_header * muh, const struct header * hd)
{
	memset(muh, 0x0, sizeof(struct mp_udp_header));

	uint16_t lfield = ntohs(hd->fields[MP_FIELD_CR_UUID]);

	muh->rc = get_rq_code(lfield);
	muh->uuid = get_uuid(lfield);
	muh->n = ntohs(hd->fields[MP_FIELD_THREAD]);
}

/*
 * This function manages to
 * create a chain of packets
 * chunked by 512 bytes each.
 */

struct packet * chunk_data(struct mp_udp_header muh, const char * data, size_t * len)
{	
	*len = *len / UDP_BLOCK_SIZE + 1;

	struct packet * packets = malloc(*len * sizeof(struct packet));
	memset(packets, 0x0, *len * sizeof(struct packet));

	size_t offset = 0;	
	size_t data_size = UDP_BLOCK_SIZE - 4;

	for(uint16_t * i = &muh.n; *i < *len; (*i)++)
	{	
		forge_udp_header(&packets[*i].header, muh);
		
		packets[*i].size = data_size;
		packets[*i].data = malloc(data_size);

		memmove(packets[*i].data, data + offset, data_size);
		offset += data_size;
	}

	return packets;
}

/*
 * This function manages to merge
 * packet' data in the correct order.
 */

char * unchunk_data(struct packet * p, size_t len)
{
	char * data = malloc(len * (UDP_BLOCK_SIZE - 4));	
	struct mp_udp_header muh;	

	for(size_t i = 0; i < len; i++)
	{
		melt_udp_header(&muh, &p[i].header);
		memmove(data + muh.n * UDP_BLOCK_SIZE, p[i].data, UDP_BLOCK_SIZE - 4);
	}

	return data;
}

/*
 * This function manages
 * to read and upload
 * data
 */

void upload(int family, const char * ip, uint16_t port, struct session * se, const char * filename)
{
	/*
	 * Reads data first
	 */

	printf("[*] preparing upload of %s to %s:%d\n", filename, ip, port);	

	int fd = open(filename, O_RDONLY);

	if(fd == -1)
	{
		printf("[-] An error occured while opening %s in read-only mode!", filename);
		return;
	}
	
	size_t len = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char * data = malloc(len + 1);
	memset(data, 0x0, len + 1);

	read(fd, data, len);
	close(fd);

	/*
	 * Then formats data
	 */	

	size_t pt_len = len;
	struct mp_udp_header muh = {UPLOAD_FILE, se->uid, 0};
	struct packet * packets = chunk_data(muh, data, &pt_len);

	/*
	 * Then opens a UDP socket
	 * and send formatted data
	 */

	int sock = socket(PF_INET, SOCK_DGRAM, 0);

	struct sockaddr addr = format_addr(family, ip, port);
	/*
	memset(&addr, 0x0, sizeof(addr));

	addr.sa_family = family;
	port = htons(port);

	memmove(addr.sa_data, &port, 2);
	inet_pton(family, ip, addr.sa_data + 6);
	*/
	char block[MP_UDP_BLOCK_SIZE];
	printf("[*] sending %ld byte(s) with %ld UDP packet(s)\n", len, pt_len);

	for(size_t k = 0; k < pt_len; k++)
	{	
		memmove(block, bufferize(&packets[k].header), 4);
		memmove(block + 4, packets[k].data, 508);
		sendto(sock, block, MP_UDP_BLOCK_SIZE, 0, &addr, sizeof(struct sockaddr));
	}

	close(sock);
}

/*
 * This function manages
 * to download and write
 * data.
 */

char * download(int family, const char * ip, uint16_t port)
{
	int sock = socket(PF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in locaddr;
	memset(&locaddr, 0x0, sizeof(locaddr));

	locaddr.sin_family = AF_INET;	
	locaddr.sin_port = htons(port);

	int status = bind(sock, (struct sockaddr *) &locaddr, sizeof(locaddr));

	if(status == -1)
	{
		printf("[-] failed to instantiate download\n");
		return NULL;
	}

	struct sockaddr from_addr = format_addr(family, ip, port);
	socklen_t sa_size = sizeof(struct sockaddr);

	printf("[*] downloading from %s:%d\n", ip, port);

	struct vector * data = make_vector(
			(void* (*)(const void*)) copy_string,
			(void (*)(void*)) free_string,
			sizeof(struct string)
			);

	struct vector * indices = make_vector(
			NULL,
			NULL,
			sizeof(size_t)
			);

	char block[MP_UDP_BLOCK_SIZE];
	size_t bytes;

	struct header hd;
	memset(&hd, 0x0, sizeof(hd));

	hd.size = 2;
	hd.fields = malloc(hd.size * FIELD_SIZE);

	struct mp_udp_header muh;	

	do {

		bytes = recvfrom(sock, block, MP_UDP_BLOCK_SIZE, 0, &from_addr, &sa_size);
		memmove(hd.fields, block, hd.size * FIELD_SIZE);
		melt_udp_header(&muh, &hd);

		push_back(indices, &muh.n);
		
		struct string * str = make_string();

		string_push_back(str, block, bytes);
		push_back(data, str);

		free_string(str);

	} while(bytes == MP_UDP_BLOCK_SIZE);

	size_t filesize = (indices->size - 1) * 508 + (bytes - 4);
	char * wo_data = malloc(filesize);

	for(size_t k = 0; k < indices->size; k++)
	{
		size_t * index = at(indices, k);
		struct string * block_str = at(data, k);

		memmove(wo_data + *index * 508, block_str->vec->data, 508);
	}

	printf("[*] %ld byte(s) downloaded\n", filesize);

	return wo_data;
}
