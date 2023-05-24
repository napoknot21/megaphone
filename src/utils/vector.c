#include "vector.h"
#include <stdio.h>

struct vector * make_vector(void* (*copy)(const void*), void (*free)(void*), size_t elem_size)
{
	struct vector * v = malloc(sizeof(struct vector));
	memset(v, 0x0, sizeof(struct vector));

	v->capacity = VECTOR_DEFAULT_CAPACITY;
	v->elem_size = elem_size;
	v->data = malloc(v->capacity * v->elem_size);

	v->copy = copy;
	v->free = free;

	return v;
}

struct vector * copy_vector(const struct vector * model)
{
	struct vector * copy = make_vector(model->copy, model->free, model->elem_size);
	capacity(copy, model->capacity);

	for(size_t i = 0; i < model->size; i++)
	{
		void * elem = at(model, i);
		push_back(copy, elem);
	}

	return copy;
}

void free_vector(struct vector * v)
{
	clear(v);
	free(v);
}

void capacity(struct vector * v, size_t cap)
{
	v->capacity = cap;
	v->data = realloc(v->data, cap * v->elem_size);
}

void clear(struct vector * v)
{
	for(size_t k = 0; v->free && k < v->size; v++)
	{
		void * el = (void*)((char*) v->data + k * v->elem_size);
		if(!el) continue;
		v->free(el);
	}

	v->size = 0;
	memset(v->data, 0x0, v->capacity);
}

void push_back(struct vector * v, const void * src)
{
	if(!v) return;

	while(v->size + 1 >= v->capacity)
	{
		v->capacity *= 2;	
		v->data = realloc(v->data, v->capacity);
	}

	void * elem = v->copy ? v->copy(src) : src;

	memmove((char*) v->data + v->size * v->elem_size, elem, v->elem_size);
	v->size++;
}

void pop_back(struct vector * v)
{
	if(!v->size) return;

	void * el = (void*)((char*) v->data + --v->size);
	v->free(el);
}

void * at(const struct vector * v, size_t i)
{
	if(i >= v->size)
	{
		return NULL;
	}

	return (void*)((char*) v->data + i * v->elem_size);
}
