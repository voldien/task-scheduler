#include "taskSch.h"
#include "internal/internal_structures.h"
#include "internal/queue.h"
#include "internal/sch.h"
#include "internal/time.h"
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <signal.h>
#include <string.h>

int translate_errno_to_sch_error(int errorCode) { return SCH_OK; }
void sch_release_scheduler_resources(schTaskSch *sch) {}

int schAllocateTaskPool(schTaskSch **pSch) {
	*pSch = (schTaskSch *)malloc(sizeof(schTaskSch));
	return 0;
}

int schCreateTaskPool(schTaskSch *sch, int cores, unsigned int flag, unsigned int maxPackagesPool) {
	unsigned int i;
	unsigned int status = SCH_OK;

	if (sch == NULL) {
		return SCH_ERROR_INVALID_SCH;
	}

	/*  Invalid argument.   */
	if (cores > schGetNumCPUCores()) {
		return SCH_ERROR_INVALID_ARG;
	}

	if (cores == -1) {
		/*	Get All the cores.	*/
		cores = schGetNumCPUCores();
	}
	sch->num = (unsigned int)cores;
	sch->flag = flag & ~(SCH_FLAG_INIT | SCH_FLAG_RUNNING);

	/*  Allocate pools. */
	sch->pool = malloc(sizeof(schTaskPool) * sch->num);
	if (sch->pool == NULL) {
		/*  Memory error and must exit.    */
		status = translate_errno_to_sch_error(errno);
		goto error;
	}
	sch->dheap = malloc(sizeof(schTaskPool *) * sch->num);
	if (sch->dheap == NULL) {
		status = translate_errno_to_sch_error(errno);
		goto error;
	}

	/*  Create signal object.   */
	sch->set = schCreateSignal();
	if (sch->set == NULL) {
		status = SCH_ERROR_UNKNOWN;
		goto error;
	}

	/*  Create internal spinlock.   */
	if (!schCreateSpinLock(&sch->spinlock)) {
		free(sch->pool);
		free(sch->dheap);
		return SCH_ERROR_SYNC_OBJECT;
	}

	status = schCreateMutex(&sch->mutex);
	if (status != SCH_OK)
		goto error;
	status = schCreateConditional(&sch->conditional);
	if (status != SCH_OK)
		goto error;

	status = schCreateBarrier(&sch->barrier);
	if (status != SCH_OK)
		goto error;
	status = schInitBarrier(sch->barrier, sch->num);
	if (status != SCH_OK)
		goto error;

	/*  Iterate through each pool and preallocate and init setup.  */
	for (i = 0; i < sch->num; i++) {

		/*  Copy the default pool pointers. */
		sch->dheap[i] = &sch->pool[i];

		/*  Allocate queue. */
		sch->pool[i].package = (schTaskPackage *)malloc(sizeof(schTaskPackage) * maxPackagesPool);
		sch->pool[i].head = 0;
		sch->pool[i].tail = 0;
		/*  Initialize queue data attributes.   */
		sch->pool[i].size = 0;
		sch->pool[i].reserved = maxPackagesPool;

		/*  Default statistics.  */
		sch->pool[i].avergeDeque = 0;
		sch->pool[i].dheapPriority = 0;

		/*  Initialize and deinitialize function pointer.   */
		sch->pool[i].init = NULL;
		sch->pool[i].deinit = NULL;
		sch->pool[i].userdata = NULL;

		/*  Thread attributes.  */
		sch->pool[i].thread = NULL;
		sch->pool[i].set = NULL;
		sch->pool[i].mutex = NULL;

		/*  */
		sch->pool[i].schRefThread = schCurrentThread();
		sch->pool[i].sch = sch;
		sch->pool[i].index = i;
		sch->pool[i].flag = 0;
	}

	/*  Set scheduler ready and initialized. */
	atomic_fetch_or(&sch->flag, SCH_FLAG_INIT);
	return status;

error: /*  Failed. Release resource in correct order if allocated. */
	sch_release_scheduler_resources(sch);
	return status;
}

int schReleaseTaskSch(schTaskSch *sch) {
	int x;
	int status = SCH_OK;

	/*  Check state of termination. */
	status = schTerminateTaskSch(sch);
	if (status != SCH_OK && status != SCH_ERROR_INVALID_STATE) {
		return status;
	}

	/*  Iterate through each pool.  */
	for (x = 0; x < sch->num; x++) {
		/*  Fetch pool pointer. */
		schTaskPool *pool = &sch->pool[x];

		// TODO add release of sync objects.
		free(pool->package);
		pool->package = NULL;
		pool->flag = 0;
	}

	/*  */
	sch_release_scheduler_resources(sch);
	return status;
}

void schSetInitCallBack(schTaskSch *sch, schUserCallBack callback) {
	int i;
	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++)
		sch->pool[i].init = callback;
}

void schSetDeInitCallBack(schTaskSch *sch, schUserCallBack callback) {
	int i;
	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++) {
		sch->pool[i].deinit = callback;
	}
}

void schSetSchUserData(schTaskSch *sch, const void *user) {
	int i;
	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++) {
		schSetPoolUserData(sch, i, user);
	}
}

void schSetPoolUserData(schTaskSch *sch, int index, const void *user) { sch->pool[index].userdata = user; }

void *schGetPoolUserData(schTaskSch *sch, int index) { return sch->pool[index].userdata; }

schTaskPool *schGetPool(schTaskSch *sch, int index) { return &sch->pool[index]; }

int schRunTaskSch(schTaskSch *sch) {
	unsigned int i;
	char buf[64] = {0};
	const int thread_name_len = 16;
	int status;

	/*  Must have been initialized before can start running.    */
	if ((sch->flag & SCH_FLAG_INIT) == 0) {
		return SCH_ERROR_INVALID_STATE;
	}

	/*  Initialize scheduler signal mask.   */
	const int mask[] = {SCH_SIGNAL_CONTINUE, SCH_SIGNAL_DONE, SCH_SIGNAL_RUNNING};
	const int nrMask = sizeof(mask) / sizeof(mask[0]);
	// TODO determine that it does not override the current masking.
	status = schSetSignalThreadMask(sch->set, nrMask, mask);
	if (status != SCH_OK)
		return SCH_ERROR_INTERNAL;

	// TODO improve status error
	schInitBarrier(sch->barrier, sch->num);

	/*  Iterate through each pool and perform final allocation.  */
	for (i = 0; i < sch->num; i++) {
		int ncoreIndex = i;

		/*  Create mutex and signal.    */
		if (!schCreateMutex(&sch->pool[i].mutex)) {
			fprintf(stderr, "failed creating mutex for pool:%d, %s", i, strerror(errno));
			return SCH_ERROR_INTERNAL;
		}
		sch->pool[i].set = schCreateSignal();

		/*  Set affinity core mapping.  */
		if (sch->flag & SCH_FLAG_NO_AFM)
			ncoreIndex = -1;

		/*  Create thread.  */
		sch->pool[i].thread = schCreateThread(ncoreIndex, schPoolExecutor, &sch->pool[i]);
		/*  Check if thread were create successfully.    */
		if (sch->pool[i].thread == NULL) {
			status = SCH_ERROR_INVALID_SCH;
			goto failed;
		}

		/*  Create thread name. */
		sprintf(buf, "task_schedule%d", i);
		buf[thread_name_len - 1] = '\0';

		/*  Set thread name - Error allowed.   */
		status = schSetThreadName(sch->pool[i].thread, buf);
	}

	/*  Iterate through each pool and start.    */
	for (i = 0; i < sch->num; i++) {
		status = schRaiseThreadSignal(sch->pool[i].thread, SCH_SIGNAL_CONTINUE);
		if (status != SCH_OK)
			return SCH_ERROR_INTERNAL;
	}

	/*  Update scheduler flag state.    */
	sch->flag |= SCH_FLAG_RUNNING; /*	Running.    */
	return SCH_OK;

failed: /*	On Failure - Release resource and status.	*/
	schTerminateTaskSch(sch);
	return status;
}

int schStopTaskSch(schTaskSch *sch, long int timeout) {
	int status;
	int i;

	/*  Must have been initialized before it can start running.    */
	if ((sch->flag & SCH_FLAG_INIT) == 0)
		return SCH_ERROR_INVALID_STATE;
	/*  Not running.    */
	if ((sch->flag & SCH_FLAG_RUNNING) == 0)
		return SCH_ERROR_INVALID_STATE;

	/*  Iterate through each pool and start.    */
	for (i = 0; i < sch->num; i++) {
		status = schRaiseThreadSignal(sch->pool[i].thread, SCH_SIGNAL_CONTINUE);
		if (status != SCH_OK)
			return SCH_ERROR_INTERNAL;
	}
	return SCH_OK;
}

int schTerminateTaskSch(schTaskSch *sch) {
	int x;
	int status = SCH_OK;
	long thread_status;

	/*  Non-initialized scheduler.  */
	if ((sch->flag & SCH_FLAG_INIT) == 0)
		return SCH_OK;

	/*  Not running.    */
	if ((sch->flag & SCH_FLAG_RUNNING) == 0) {
		return SCH_OK;
	}

	/*  Wait in till scheduler is finished with all tasks. */
	status = schWaitTask(sch);

	/*  Iterate through each pool.  */
	for (x = 0; x < sch->num; x++) {
		schTaskPool *pool = &sch->pool[x];

		/*  If thread has been created. */
		if (pool->thread) {
			status &= schRaiseThreadSignal(pool->thread, SIGTERM);
			//			status &= schRaiseThreadSignal(pool->thread, SCH_SIGNAL_QUIT);

			/*  Wait in till thread has terminated. */
			status &= schWaitThread(pool->thread, &thread_status);
			// status &= schDeleteThread(pool->thread);
			pool->thread = NULL;
			pool->schRefThread = NULL;

			/*  */
			if (pool->mutex)
				status &= schDeleteMutex(pool->mutex);
			if (pool->set)
				status &= schDeleteSignal(pool->set);

			/*  */
			pool->mutex = NULL;
			pool->set = NULL;
		}
	}

	/*  Update scheduler state. */
	atomic_fetch_and(&sch->flag, ~SCH_FLAG_RUNNING);

	/*  */
	return status ? SCH_OK : SCH_ERROR_UNKNOWN;
}

int schSubmitTask(schTaskSch *sch, schTaskPackage *package, schTaskPool *pPool) {
	schTaskPool *pool;

	if ((sch->flag & SCH_FLAG_RUNNING) == 0 || (sch->flag & SCH_FLAG_INIT) == 0) {
		return SCH_ERROR_INVALID_STATE;
	}

	/*  Validate argument.  */
	if (package == NULL || package->callback == NULL) {
		return SCH_ERROR_INVALID_ARG;
	}

	/*	Determine which pool has least work ahead.*/
	schHeapify(sch);
	if (pPool == NULL) {
		pool = sch->dheap[0];
	} else {
		pool = pPool;
	}

	/*  Full queue. */
	if (pool->size >= pool->reserved) {
		return SCH_ERROR_POOL_FULL;
	}

	/*  Set pool index. */
	package->index = pool->index;

	/*  Copy package and update queue.  */
	schQueueMutexEnDeQueue(pool, 0, package);

	// TODO resolve.
	pool->dheapPriority += 1000;

	/*  If pool is finished.    */
	if (pool->size <= 1 && pool->flag & SCH_POOL_SLEEP) {
		if (!schRaiseThreadSignal(pool->thread, SCH_SIGNAL_CONTINUE))
			return SCH_ERROR_SIGNAL;
		schInitBarrier(sch->barrier, sch->num);
	}
	return SCH_OK;
}

int schClearTask(schTaskSch *sch, schTaskPool *pool) { return SCH_OK; }

int schClearAllTask(schTaskSch *sch) { return SCH_OK; }

int schWaitTask(schTaskSch *sch) { return schWaitTaskWait(sch, -1); }

int schWaitTaskWait(schTaskSch *sch, long wait) {
	int i;

	/*  Check if valid scheduler.   */
	if ((sch->flag & SCH_FLAG_RUNNING) == 0) {
		return SCH_ERROR_INVALID_STATE;
	}

	/*  */ // TODO add support for wait utilization.
	schMutexLock(sch->mutex);
	while (!(sch->flag & SCH_FLAG_RUNNING)) {
		schConditionalWait(sch->conditional, sch->mutex);
	}
	schMutexUnLock(sch->mutex);

	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++) {
		if (sch->pool[i].flag & SCH_POOL_SLEEP && sch->pool[i].size <= 0)
			continue;
		else {
			/*  Wait and check every milliseconds reset. */
			if (schSignalWaitTimeOut(sch->set, (long int)1E7L) < 0) {
				fprintf(stderr, "signal wait failed: %s", strerror(errno));
			}
			i = -1;
		}
	}
	return SCH_OK;
}

void *schQueueMutexEnDeQueue(schTaskPool *taskPool, int dequeue, void *enqueue) {
	schTaskPackage *package = enqueue;

	schTaskSch *sch = taskPool->sch;
	schLockSpinLock(sch->spinlock);
	if (dequeue) {
		taskPool->size--;
		package = &taskPool->package[taskPool->head];
		taskPool->head = (taskPool->head + 1) % taskPool->reserved;
	} else {
		memcpy(&taskPool->package[taskPool->tail], package, sizeof(*package));
		taskPool->tail = (taskPool->tail + 1) % taskPool->reserved;
		taskPool->size++;
	}
	schUnlockSpinLock(sch->spinlock);

	return package;
}

static void setPoolRunning(schTaskPool *pool) {
	pool->flag |= SCH_POOL_RUNNING;
	pool->flag = pool->flag & ~SCH_POOL_SLEEP;
}

static void setPoolIdle(schTaskPool *pool) { pool->flag = (pool->flag & ~SCH_POOL_RUNNING) | SCH_POOL_SLEEP; }
static void setPoolTerminated(schTaskPool *pool) {
	pool->flag = (pool->flag & ~(SCH_POOL_RUNNING | SCH_POOL_SLEEP)) | SCH_POOL_TERMINATE;
}

const char *schErrorMsg(int errMsg) {
	static const char *msgErr[] = {
		"unknown error",			/*  SCH_ERROR_UNKNOWN : 0   */
		"invalid argument",			/*  SCH_ERROR_INVALID_ARG : -1  */
		"invalid schedular object", /*  SCH_ERROR_INVALID_SCH : -2  */
		"schedular/pool bad state", /*  SCH_ERROR_INVALID_STATE */
		"internal error",			/*  SCH_ERROR_INTERNAL  */
		"pool queue is full",		/*  SCH_ERROR_POOL_FULL */
		"internal signal error",	/*  SCH_ERROR_SIGNAL    */
		"Synchronization error",	/*  SCH_ERROR_SYNC_OBJECT    */
		"Timeout error",			/*  SCH_ERROR_TIMEOUT    */
		"Busy error",				/*  SCH_ERROR_BUSY    */
		"No Memory",				/*  SCH_ERROR_NOMEM    */
		"OS lacking resources",		/*  SCH_ERROR_LACK_OF_RESOURCES    */
		"Permission denied",		/*  SCH_ERROR_PERMISSION_DENIED    */
	};

	/*  Check and the error message and determine if error code within the error array size.    */
	if (errMsg == SCH_OK) {
		return "no error";
	}
	if (errMsg > SCH_OK) {
		return NULL;
	}

	if (errMsg < SCH_ERROR_PERMISSION_DENIED) {
		return NULL;
	}

	return msgErr[errMsg * -1];
}
