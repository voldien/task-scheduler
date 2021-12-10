/*
 *	Task scheduler for uniform task processing in user space.
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
#ifndef _TASK_SCH_INTERNAL_STRUCTURES_H_
#define _TASK_SCH_INTERNAL_STRUCTURES_H_ 1
#include "../taskSch.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief TaskPool.
 *
 */
typedef struct sch_task_pool_t {

	/**
	 * Thread associated with the pool.
	 */
	schThread *thread;
	/**
	 * Reference to the scheduler thread
	 *
	 */
	schThread *schRefThread;

	/*  Thread race condition variables.    */
	void *mutex; /*	Mutex.	*/ // TODO remove.
	void *set;				   /*	Thread signal.  */

	/*  Task Queue.  */ // TODO consider using linked list for adding support for task migration.
	// TODO add atomic for queue strucuture if posssible and faster than spinlock..
	unsigned int size;		 /*  Size of number of elements. */
	unsigned int tail;		 /*  End of the queue.   */
	unsigned int head;		 /*  Start of the queue. */
	unsigned int reserved;	 /*  Number of allocate task packages.   */
	schTaskPackage *package; /*  */

	/*  User and init and release functions callbacks.  */
	void *userdata;	/*  User data.  */
	schUserCallBack init;	/*  Init user callback function.    */
	schUserCallBack deinit; /*  DeInit user callback function.  */

	/*  Statistics for handling next task to execute. */
	int avergeDeque;		/*  Average package dequeue per time unit.  */
	long int dheapPriority; /*  */

	/*  State and scheduler.    */
	void *sch;			/*  Scheduler associated with.  */
	unsigned int index; /*  Affinity index. */
	atomic_uint flag;	/*  Pool status flag.   */
} schTaskPool;

/**
 * @brief Task scheduler main struct container.
 */
typedef struct sch_task_scheduler_t {
	/**
	 * @brief
	 *
	 */
	size_t num; /*  Number of pools.    */
	/**
	 * @brief
	 *
	 */
	schTaskPool *pool; /*  Pools.  */
	/**
	 * @brief
	 *
	 */
	atomic_uint flag; /*  State/Status Flags. */
	/**
	 * @brief
	 *
	 */
	schTaskPool **dheap; /*  Priority queue. */
	/**
	 * @brief
	 *
	 */
	schSpinLock *spinlock; /*  Spin lock.  */
	/**
	 * @brief
	 *
	 */
	schMutex *mutex;
	/**
	 * @brief
	 *
	 */
	schConditional *conditional;
	/**
	 * @brief
	 *
	 */
	schBarrier *barrier;
	/**
	 * @brief
	 *
	 */
	schSignalSet *set; /*  Signal listening mask.  */
} schTaskSch;

#ifdef __cplusplus
}
#endif

#endif
