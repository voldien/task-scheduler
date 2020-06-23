/**
	Task scheduler for uniform processing in user space.
	Copyright (C) 2015  Valdemar Lindberg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef _SCH_INTERNAL_SCH_H_
#define _SCH_INTERNAL_SCH_H_ 1
#include "taskSch.h"
#include "queue.h"
#include "poolAllocator.h"

#ifdef __cplusplus
extern "C"{
#endif


/**
 * Perform queue enqueue/deqeue that will prevent
 * race condition.
 * @param dequeue non-zero will perform dequeue. otherwise enqueue.
 * @param enqueue enqueue data pointer.
 * @return NULL if enqueue, otherwise data from the dequeue.
 */
extern TASH_SCH_EXTERN void *schQueueMutexEnDeQueue(schTaskPool *taskPool, int dequeue, void *enqueue); //TODO rename.

/**
 * Perform priority queue on the pool
 * @param sch non-null scheduler object.
 */
extern TASH_SCH_EXTERN void schHeapify(schTaskSch *sch);

/**
 * Pool thread execution environment.
 * @param handle user specified data.
 * @return NULL
 */
extern TASH_SCH_EXTERN void *schPoolExecutor(void *handle);

#ifdef __cplusplus
}
#endif


#endif