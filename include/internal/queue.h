/*
 *	Task scheduler for uniform processing in user space.
 *	Copyright (C) 2015  Valdemar Lindberg
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _SCH_QUEUE_STRUCTURES_H_
#define _SCH_QUEUE_STRUCTURES_H_ 1
#include "poolAllocator.h"
#include <stdatomic.h>
#ifdef __cplusplus
extern "C" {
#endif

// atomic_uintptr_t
/**
 * @brief
 *
 */
typedef struct linked_node {
	atomic_int index;		  /*  */
	struct linked_node *next; /*  */
	struct linked_node *prev; /*  */
} LinkedNode;

/**
 * @brief
 *
 */
typedef struct queue_sync_double_linked_t {
	atomic_int size;		  /*  Size of number of elements. */
	atomic_int tail;		  /*  End of the queue.   */
	atomic_int head;		  /*  Start of the queue. */
	atomic_int reserved;	  /*  Number of allocate task packages.   */
	int typeSize;			  /*  Object size in bytes.   */
	void *data; /*  */		  // TODO maybe remove
	LinkedNode *nodes; /*  */ // TODO maybe remove
	SchPool pool;			  /*  Allocator pool for all the node objects.    */
} DLQueue;


extern int schCreateQueue(DLQueue *DLQueue, unsigned int nr_reserved, unsigned int type_size);

extern int schDequeue(DLQueue *DLQueue);
extern int schEnqueue(DLQueue *DLQueue);

extern int schClear(DLQueue *DLQueue);
extern int schQueueGetSize(DLQueue *DLQueue);

extern int schQueueGetReserved(DLQueue *DLQueue);

#ifdef __cplusplus
}
#endif

#endif
