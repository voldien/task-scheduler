#include "internal/queue.h"
#include <malloc.h>
int schCreateQueue(DLQueue *DLQueue, unsigned int nr_reserved, unsigned int type_size) {
	DLQueue->size = 0;
	DLQueue->tail = 0;
	DLQueue->head = 0;
	DLQueue->reserved = nr_reserved;
	DLQueue->typeSize = type_size;
	schInitPool(&DLQueue->pool, nr_reserved, sizeof(struct linked_node));
	size_t queue_data_size = (size_t)type_size * (size_t)nr_reserved;
	DLQueue->data = malloc(queue_data_size);

	return 0;
}

int schDequeue(DLQueue *DLQueue) { return 0; }

int schEnqueue(DLQueue *DLQueue) { return 0; }

int schClear(DLQueue *DLQueue) { return 0; }

int schQueueGetSize(DLQueue *DLQueue) { return 0; }

int schQueueGetReserved(DLQueue *DLQueue) { return 0; }
