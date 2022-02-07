#include "internal/poolAllocator.h"
#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>

SchPool *schInitPool(SchPool *alloc, unsigned int num, unsigned int itemsize) {
	unsigned char *tmp;
	unsigned int i;
	const int size = (itemsize + sizeof(SchPool)); /*	Total size of each node.	*/

	/*	Allocate number pool nodes.	*/
	alloc->pool = calloc(num, size);
	alloc->num = num;
	alloc->itemsize = itemsize;
	assert(alloc->pool);

	/*	Create init pool chain.	*/
	tmp = (unsigned char *)alloc->pool;
	for (i = 0; i < num; i++) {
		((SchPoolNode *)tmp)->next = (SchPoolNode *)(tmp + sizeof(SchPoolNode) + itemsize);
		tmp += itemsize + sizeof(SchPoolNode);
	}

	/*	Terminator of the pool.	*/
	tmp -= itemsize + sizeof(SchPoolNode);
	/*	Assign end item indicates the the last item in the pool chain.	*/
	((SchPoolNode *)tmp)->next = NULL;

	return alloc;
}

SchPool *schPoolCreate(unsigned int num, unsigned int itemsize) {

	SchPool *alloc;

	/*	Allocate pool descriptor.	*/
	alloc = malloc(sizeof(SchPool));
	assert(alloc);

	return schInitPool(alloc, num, itemsize);
}

void *schPoolObtain(SchPool *allocator) {

	SchPoolNode *tmp;
	void *block;

	/*	Check if empty.	*/
	if (allocator->pool->next == NULL) {
		return NULL;
	}

	/*	Get next element and assigned new next element.	*/
	tmp = allocator->pool->next;
	/*	Update the chain with the new next.	*/
	allocator->pool->next = tmp->next;

	/*	Get data block from the pool item.	*/
	block = tmp->data;
	return block;
}

void *schPoolReturn(SchPool *allocator, void *data) {

	SchPoolNode *tmp;

	/*	Decrement with size of a pointer
	 *	to get pointer for the next element.*/
	tmp = (SchPoolNode *)(((char *)data) - sizeof(void *));

	/*	Update next value.	*/
	tmp->next = allocator->pool->next;
	allocator->pool->next = tmp;

	return tmp;
}

void *schPoolResize(SchPool *allocator, unsigned int num, unsigned int itemsize) { return NULL; }

unsigned int schPoolNumNodes(const SchPool *pool) { return pool->num; }

unsigned int schPoolItemSize(const SchPool *pool) { return pool->itemsize; }

int schPoolGetIndex(const SchPool *pool, const void *data) {

	const uint8_t *offset = (const uint8_t *)((const uint8_t *)data - (const uint8_t *)pool->pool);
	return ((uintptr_t)offset) / pool->itemsize;
}

static inline void *sntPoolItemByIndex(SchPool *pool, unsigned int index) {
	return ((uint8_t *)pool->pool) + ((pool->itemsize + sizeof(void *)) * index + sizeof(void *));
}

void schPoolFree(SchPool *pool) {

	/*	Release memory.	*/
	free(pool->pool);
	free(pool);
}
