#include"taskSch.h"
#include<malloc.h>
#include<assert.h>
#include<string.h>
#include <errno.h>

schTaskSch *schCreateTaskPool(schTaskSch *sch, int cores, unsigned int flag, unsigned int maxPackagesPool) {
	unsigned int i;

	/*  */
	if(cores > schGetNumCPUCores())
		return NULL;

	if (cores == -1)
		cores = schGetNumCPUCores();
	sch->num = (unsigned int)cores;
	sch->flag = flag;

	/*  Allocate pools. */
	sch->pool = malloc(sizeof(schTaskPool) * sch->num);
	assert(sch->pool);
	sch->dheap = malloc(sizeof(schTaskPool*) * sch->num);

	sch->set = schCreateSignal();

	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++) {

		/*  Copy the default pool pointers. */
		sch->dheap[i] = &sch->pool[i];

		/*  Allocate queue. */
		sch->pool[i].package = (schTaskPackage *) malloc(sizeof(schTaskPackage) * maxPackagesPool);
		sch->pool[i].head = 0;
		sch->pool[i].tail = 0;

		/*  Initialize queue data attributes.   */
		sch->pool[i].size = 0;
		sch->pool[i].reserved = maxPackagesPool;
		sch->pool[i].index = i;

		/*  Default statistics.  */
		sch->pool[i].avergeDeque = 0;
		sch->pool[i].dheapPriority = 0;

		/*  Initialize and deinitialize function pointer.   */
		sch->pool[i].init = NULL;
		sch->pool[i].deinit = NULL;

		/*  Thread attributes.  */
		sch->pool[i].thread = NULL;
		sch->pool[i].set = NULL;
		sch->pool[i].mutex = NULL;

		sch->pool[i].schThread = schCurrentThread();
	}

	sch->flag |= SCH_FLAG_INIT;
	return sch;
}


int schReleaseTaskSch(schTaskSch *sch){

	int x;

	schTerminateTaskSch(sch);

	/*  Iterate through each pool.  */
	for (x = 0; x < sch->num; x++) {
		schTaskPool* pool = &sch->pool[x];

		/*  Empty pool size.    */
		free(pool->package);
		pool->package = NULL;
		pool->flag = 0;
	}

	/*  Release pool and heap.  */
	free(sch->pool);
	free(sch->dheap);
	free(sch->set);

	/*  */
	sch->dheap = NULL;
	sch->pool = NULL;
	sch->set = NULL;

	return SCH_OK;
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
	for (i = 0; i < sch->num; i++)
		sch->pool[i].deinit = callback;
}

void schSetSchUserData(schTaskSch* sch, const void* user) {
	int i;
	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++)
		schSetPoolUserData(sch, i, user);
}

void schSetPoolUserData(schTaskSch *sch, int index, void *user) {
	sch->pool[index].userdata = user;
}

void *schGetPoolUserData(schTaskSch *sch, int index) {
	return sch->pool[index].userdata;
}

int schRunTaskSch(schTaskSch *sch) {
	unsigned int i;
	char buf[64] = {0};

	/*  Must have been initialized before can start running.    */
	if ((sch->flag & SCH_FLAG_INIT) == 0)
		return SCH_ERROR_INVALID_STATE;

	/*  Initialize schedular signal mask.   */
	const int mask[] = {SCH_SIGNAL_CONTINUE, SCH_SIGNAL_DONE, SCH_SIGNAL_RUNNING};
	const int nrMask = sizeof(mask) / sizeof(mask[0]);
	if(schSetSignalThreadMask(sch->set, nrMask, mask) <= 0) {
		return SCH_ERROR_INTERNAL;
	}

	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++) {

		/*  Create mutex and signal.    */
		if (!schCreateMutex(&sch->pool[i].mutex)) {
			fprintf(stderr, "failed creating mutex for pool:%d, %s", i, strerror(errno));
			return SCH_ERROR_INTERNAL;
		}
		sch->pool[i].set = schCreateSignal();

		/*  Create thread.  */
		sch->pool[i].thread = schCreateThread(i, schPoolExecutor, &sch->pool[i]);
		assert(sch->pool[i].thread);

		/*  Check if thread were create successfully.    */
		if (sch->pool[i].thread != NULL) {
			/*  Create thread name. */
			sprintf(buf, "task_schedule%d", i);
			buf[15] = '\0';

			/*  Set thread name.    */
			schSetThreadName(sch->pool[i].thread, buf);
		} else {
			/*  Error occurred!  */
			fprintf(stderr, "Failed to create thread:%d\n", i);
			return SCH_ERROR_INTERNAL;
		}
	}


	/*  Iterate through each pool and start.    */
	for (i = 0; i < sch->num; i++) {
		if(schRaiseThreadSignal(sch->pool[i].thread, SCH_SIGNAL_CONTINUE) <= 0)
			return SCH_ERROR_INTERNAL;
	}

	/*  Update scheduler flag state.    */
	sch->flag |= SCH_FLAG_RUNNING;    /*	Running.    */
	return SCH_OK;
}

int schTerminateTaskSch(schTaskSch *sch) {
	int x;

	/*  Non-initialized scheduler.  */
	if(sch->flag & SCH_FLAG_INIT == 0)
		return SCH_ERROR_INVALID_SCH;

	/*  Not running.    */
	if(sch->flag & SCH_FLAG_RUNNING == 0)
		return SCH_ERROR_INVALID_STATE;

	/*  */
	schWaitTask(sch);

	/*  Iterate through each pool.  */
	for (x = 0; x < sch->num; x++) {
		schTaskPool* pool = &sch->pool[x];

		/*  If thread has been created. */
		if(pool->thread){
			schRaiseThreadSignal(pool->thread, SCH_SIGNAL_QUIT);

			/*  */
			schWaitThread(pool->thread);
			schDeleteThread(pool->thread);
			pool->thread = NULL;
			pool->schThread = NULL;

			/*  */
			if(pool->mutex) {
				schDeleteMutex(pool->mutex);
				free(pool->set);
			}

			pool->mutex = NULL;
			pool->set = NULL;
		}
	}


	return SCH_OK;
}


int schSubmitTask(schTaskSch *sch, schTaskPackage *package, schTaskPool *pPool) {

	schTaskPool *pool;

	/*  Validate argument.  */
	if(package == NULL || package->callback == NULL)
		return SCH_ERROR_INVALID_ARG;

	/*	Determine which pool has least work ahead.*/
	schHeapify(sch);
	if(pPool == NULL)
		pool = sch->dheap[0];
	else
		pool = pPool;

	/*  Full queue. */
	if(pool->size >= pool->reserved)
		return SCH_ERROR_POOL_FULL;

	/*  Copy package and update queue.  */
	schQueueMutexEnDeQueue(pool, 0, package);

	pool->dheapPriority += 1000;

	/*  If pool is finished.    */
	if (pool->size <= 1 && pool->flag & SCH_POOL_SLEEP) {
		if(!schRaiseThreadSignal(pool->thread, SCH_SIGNAL_CONTINUE))
			return SCH_ERROR_INTERNAL;
	}
	return SCH_OK;
}

int schWaitTask(schTaskSch *sch) {
	int i;
	/*  Iterate through each pool.  */
	for (i = 0; i < sch->num; i++) {
		if(sch->pool[i].flag & SCH_POOL_SLEEP && sch->pool[i].size <= 0)
			continue;
		else{
			/*  Wait and check every milliseconds reset. */
			schSignalWaitTimeOut(sch->set, (long int)1E7L);
			i = -1;
		}
	}
	return SCH_OK;
}

void* schQueueMutexEnDeQueue(schTaskPool *taskPool, int dequeue, void *enqueue){
	schTaskPackage* package = enqueue;

	schPoolLock(taskPool);
	if(dequeue){
		taskPool->size--;
		package = &taskPool->package[taskPool->head];
		taskPool->head = (taskPool->head + 1) % taskPool->reserved;
	}else{
		memcpy(&taskPool->package[taskPool->tail], package, sizeof(*package));
		taskPool->tail = (taskPool->tail + 1) % taskPool->reserved;
		taskPool->size++;
	}
	schPoolUnLock(taskPool);
	return package;
}

static void setPoolRunning(schTaskPool* pool){
	pool->flag |= SCH_POOL_RUNNING;
	pool->flag = pool->flag & ~SCH_POOL_SLEEP;
}

static void setPoolIdle(schTaskPool* pool){
	pool->flag = (pool->flag & ~SCH_POOL_RUNNING) | SCH_POOL_SLEEP;
}

void *schPoolExecutor(void *handle) {

	/*	*/
	int signal;
	const unsigned int SigQuit = SCH_SIGNAL_QUIT;
	const long int timeRes = schTimeResolution();
	long int taskInvoke;
	schTaskPool *pool = handle;
	const unsigned int index = pool->index;
	assert(handle);

	/*  Initialize thread signal mask.   */
	const int mask[] = {SCH_SIGNAL_CONTINUE, SCH_SIGNAL_DONE, SCH_SIGNAL_RUNNING, SCH_SIGNAL_QUIT};
	const int nrMask = sizeof(mask) / sizeof(mask[0]);
	if(schSetSignalThreadMask(pool->set, nrMask, mask) <= 0)
		goto error;

	/*  Wait in till all thread has been executed and is ready.   */
	while(schSignalWait(pool->set) != SCH_SIGNAL_CONTINUE){}

	/*	Initialize callback */
	if (pool->init)
		pool->init(pool);

	/*	Main iterative loop.	*/
	do {
		if (pool->size > 0) {
			schCallback callback;
			schTaskPackage* package;

			/*  Get next package and update queue.  */
			package = schQueueMutexEnDeQueue(pool, 1, NULL);

			/*  Extract package and callback and update queue.  */
			callback = package->callback;
			package->index = index;

			/*  Invoke task callback.   */
			taskInvoke = schGetTime();
			callback(package);
			taskInvoke = schGetTime() - taskInvoke;

			/*  Update pool priority queue key. */
			pool->avergeDeque = 0;
			pool->dheapPriority = taskInvoke;
		} else {

			/*  No tasks.   */
			pool->dheapPriority = 0;

			/*  Set pool in idle state. */
			setPoolIdle(pool);

			/*	Send signal to main thread, that pool is finished.	*/
			schRaiseThreadSignal(pool->schThread, SCH_SIGNAL_DONE);

			/*  Wait in till additional packages has been added and continue signal has been issued.    */
			do {

				/*  Wait in till the signal from the scheduler gives signal to continue.    */
				signal = schSignalWait(pool->set);
				if (signal == SigQuit) {
					fprintf(stderr, "Quitting task schedule thread of core %d\n", pool->index);
					goto error;
				}

			} while (signal != SCH_SIGNAL_CONTINUE);

			/*	Set pool thread as running.	*/
			setPoolRunning(pool);
		}

	} while (1);  /*  */

	error:    /*	failure.	*/

	/*	clean up.	*/
	if (pool->deinit)
		pool->deinit(pool);

	/*  Update flag status. */
	pool->flag = (pool->flag & ~(SCH_POOL_RUNNING | SCH_POOL_SLEEP)) | SCH_POOL_TERMINATE;
	return NULL;
}

const char* schErrorMsg(int errMsg){
	static const char* msg[] = {
			"no error.",                /*  SCH_OK */
			"unknown error",            /*  SCH_ERROR_UNKNOWN  */
			"invalid argument",         /*  SCH_ERROR_INVALID_ARG   */
			"invalid schedular object", /*  SCH_ERROR_INVALID_SCH   */
			"schedular/pool bad state", /*  SCH_ERROR_INVALID_STATE */
			"internal error",           /*  SCH_ERROR_INTERNAL  */
			"pool queue is full"        /*  SCH_ERROR_POOL_FULL */
	};
	if(errMsg > 0)
		return "Invalid error msg";
	else
		return msg[errMsg * -1];
}