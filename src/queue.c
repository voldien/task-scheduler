#include"internal/queue.h"

int schCreateQueue(DLQueue *DLQueue, unsigned int nr_reserved, unsigned int type_size) {
	DLQueue->size = 0;
	DLQueue->tail = 0;
	DLQueue->head = 0;
	DLQueue->reserved = nr_reserved;
	DLQueue->typeSize = type_size;
	schInitPool(&DLQueue->pool, nr_reserved, sizeof(struct linked_node));
	DLQueue->data = malloc(type_size  * nr_reserved);
}

int schDequeue(DLQueue *DLQueue) {

}

int schEnqueue(DLQueue *DLQueue) {

}

int schClear(DLQueue *DLQueue) {

}

int schQueueGetSize(DLQueue *DLQueue) {

}

int schQueueGetReserved(DLQueue *DLQueue) {

}




