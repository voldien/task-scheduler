#include "internal/sch.h"
#include "internal/internal_structures.h"
#include "internal/time.h"
#include "taskSch.h"
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>

/*  State functions of the pool.   */
static inline void setPoolRunning(schTaskPool *pool) {
	atomic_fetch_xor(&pool->flag, SCH_POOL_SLEEP | SCH_FLAG_RUNNING);
}

static inline void setPoolIdle(schTaskPool *pool) { atomic_fetch_xor(&pool->flag, SCH_POOL_RUNNING | SCH_POOL_SLEEP); }

static inline void setPoolTerminated(schTaskPool *pool) {
	atomic_fetch_xor(&pool->flag, SCH_POOL_RUNNING | SCH_POOL_SLEEP | SCH_POOL_TERMINATE);
}

static inline int isRunning(schTaskPool *pool) { return atomic_load(&pool->flag) & SCH_POOL_RUNNING; }

static void hdl(int sig, siginfo_t *siginfo, void *context) {
	printf("Sending PID: %ld, UID: %ld\n", (long)siginfo->si_pid, (long)siginfo->si_uid);

	// siglongjmp(jmpbuf, 1);
}

void *schPoolExecutor(void *handle) {

	/*	*/
	int signal, status;
	const unsigned int SigQuit = SCH_SIGNAL_QUIT;
	const long int timeRes = schTimeResolution();
	long int taskInvoke;
	jmp_buf jump;

	assert(handle);

	/*  Get pool attributes.    */
	schTaskPool *pool = handle;
	const schTaskSch *sch = pool->sch;
	const unsigned int index = pool->index;

	/*  Initialize thread signal mask.   */
	const int mask[] = {SCH_SIGNAL_CONTINUE, SCH_SIGNAL_DONE, SCH_SIGNAL_RUNNING, SCH_SIGNAL_QUIT, SIGTERM};
	const int nrMask = sizeof(mask) / sizeof(mask[0]);
	if (schSetSignalThreadMask(pool->set, nrMask, mask) <= 0)
		goto error;

	/*  */
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_sigaction = &hdl;
	act.sa_flags = SA_SIGINFO;
	status = sigaction(SIGINT, &act, NULL);

	/*  Align synchronization of all task scheduler pool.   */
	while (isRunning(pool))
		schConditionalWait(sch->conditional, pool->mutex);

	/*  Wait in till all thread has been executed and is ready.   */
	// while(schSignalWait(pool->set) != SCH_SIGNAL_CONTINUE){}

	// schWaitBarrier(sch->barrier);

	/*	Initialize callback */
	if (pool->init)
		pool->init(pool);

	/*  */
	sigsetjmp(jump, 1);

	/*	Main iterative loop.	*/
	do {
		if (pool->size > 0) {
			schCallback callback;
			schTaskPackage *package;

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

			// schWaitBarrier(sch->barrier);
			/*  Set pool in idle state. */
			setPoolIdle(pool);
			schMutexUnLock(sch->mutex);
			schConditionalSignal(sch->conditional);

			/*  Set pool in idle state. */
			setPoolIdle(pool);

			/*	Send signal to main thread, that pool is finished.	*/
			schRaiseThreadSignal(pool->schRefThread, SCH_SIGNAL_DONE);
			// schWaitBarrier()

			/*  Wait in till additional packages has been added and continue signal has been issued.    */
			do {

				/*  Wait in till the signal from the scheduler gives signal to continue.    */
				signal = schSignalWait(pool->set);
				switch (signal) {
				case SIGQUIT:
					break;
				case SIGINT:
					break;
				case SIGTERM:
					goto done;
				default:
					continue;
				}
				if (signal == SigQuit) {
					fprintf(stderr, "Quitting task schedule thread of core %d\n", pool->index);
					goto done;
				}

			} while (signal != SCH_SIGNAL_CONTINUE);

			/*	Set pool thread as running.	*/
			setPoolRunning(pool);
		}

	} while (1); /*  */

error: /*	failure.	*/
done:  /*  Finished.   */

	/*	clean up.	*/
	if (pool->deinit)
		pool->deinit(pool);

	/*  Update flag status. */
	setPoolTerminated(pool);
	return (void *)status;
}
