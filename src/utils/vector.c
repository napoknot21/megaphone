#include "vector.h"

struct vector * make_vector(void (*free)(void*), size_t elem_size)
{
	struct vector * v = malloc(sizeof(struct vector));
	memset(v, 0x0, sizeof(struct vector));

	v->capacity = VECTOR_DEFAULT_CAPACITY;
	v->elem_size = elem_size;
	v->data = malloc(v->capacity * v->elem_size);

	v->free = free;

	return v;
}

void free_vector(struct vector * v)
{
	clear(v);
	free(v);
}

void clear(struct vector * v)
{
	for(size_t k = 0; k < v->size; v++)
	{
		void * el = (void*)((char*) v->data + k * v->elem_size);
		if(!el) continue;
		v->free(el);
	}
}

void push_back(struct vector * v, void * src)
{
	while(v->size + 1 >= v->capacity)
	{
		v->capacity *= 2;
		v->data = realloc(v->data, v->capacity);
	}
	
	memmove((char*) v->data + v->size, src, v->elem_size);
	v->size++;
}

void pop_back(struct vector * v)
{
	if(!v->size) return;

	void * el = (void*)((char*) v->data + v->size - 1);
	v->free(el);
}
