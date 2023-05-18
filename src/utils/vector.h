#ifndef VECTOR_H
#define VECTOR_H

#include <string.h>
#include <stdlib.h>

#define VECTOR_DEFAULT_CAPACITY 8

struct vector
{
	void * data;

	void* (*copy)(void*);
	void (*free)(void*);

	size_t elem_size;
	size_t capacity;
	size_t size;
};

struct vector * make_vector(void* (*)(void*), void (*)(void*), size_t);
void free_vector(struct vector*);

void push_back(struct vector*, void*);
void pop_back(struct vector*);

void clear(struct vector*);

void * at(struct vector*, size_t);

#endif
