#ifndef _TASK_SCH_INTERNAL_STRUCTURES_H_
#define _TASK_SCH_INTERNAL_STRUCTURES_H_ 1
#include"../taskSch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Scheduler task structure.
 */

typedef struct sch_task_package_t {
	/*  Package data.   */
	atomic_uint flag;	/*  Package flag.   */
	unsigned int index; /*  Pool index. */
	/*  User data.  */
	schCallback callback; /*  Function callback.  */
	// TODO determine to add long support.
	unsigned int size;	 /*  Size parameter. */
	unsigned int offset; /*  Offset. */
	void *begin;		 /*  Start pointer.  */
	void *end;			 /*  End pointer.    */
	void *puser;		 /*  User data.  */
} schTaskPackage;

// TODO relocate t the source code

/**
 * Pool structure.
 */
typedef struct sch_task_pool_t {

	/*  Threading.  */
	schThread *thread;		 /*  Thread associated with the pool.    */
	schThread *schRefThread; /*  Scheduler thread.   */

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
	const void *userdata;	/*  User data.  */
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
 * Task scheduler main struct container.
 */
typedef struct sch_task_scheduler_t {
	unsigned int num;	   /*  Number of pools.    */
	schTaskPool *pool;	   /*  Pools.  */
	atomic_uint flag;	   /*  State/Status Flags. */
	schTaskPool **dheap;   /*  Priority queue. */
	schSpinLock *spinlock; /*  Spin lock.  */
	schMutex *mutex;
	schConditional *conditional;
	schBarrier *barrier;
	schSignalSet *set; /*  Signal listening mask.  */
} schTaskSch;

#ifdef __cplusplus
}
#endif

#endif
