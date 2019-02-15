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
#ifndef _CORE_TASK_SCH_H_
#define _CORE_TASK_SCH_H_ 1

#ifdef __cplusplus
extern "C"{
#endif

/**
 * Task scheduler option flags.
 */
#define SCH_FLAG_NO_AFM     (unsigned int)0x80000000    /*  Disable affinity mapping.    */

/**
 * Task scheduler internal flags.
 */
#define SCH_FLAG_INIT       (unsigned int)0x1           /*  Scheduler has been initialized.    */
#define SCH_FLAG_RUNNING    (unsigned int)0x2           /*  Scheduler is in running mode.   */

/**
 * Pool status flags.
 */
#define SCH_POOL_TERMINATE  (unsigned int)0x1   /*  Pool has been terminated.   */
#define SCH_POOL_RUNNING    (unsigned int)0x2   /*  Pool is in running mode.    */
#define SCH_POOL_SLEEP      (unsigned int)0x4   /*  Pool is in sleep mode.  */

/**
 * Task scheduler signals.
 */
#define SCH_SIGNAL_IDLE     (unsigned int)(schBaseSignal() + 0) /*  */
#define SCH_SIGNAL_RUNNING  (unsigned int)(schBaseSignal() + 1) /*  */
#define SCH_SIGNAL_DONE     (unsigned int)(schBaseSignal() + 2) /*  */
#define SCH_SIGNAL_CONTINUE (unsigned int)(schBaseSignal() + 3) /*  */
#define SCH_SIGNAL_QUIT     (unsigned int)(schBaseSignal() + 4) /*  */

/**
 * Error messages.
 */
#define SCH_OK                  (int)1      /*  No error.   */
#define SCH_ERROR_UNKNOWN       (int)0      /*  Error unknown.   */
#define SCH_ERROR_INVALID_ARG   (int)-1     /*  Invalid argument.   */
#define SCH_ERROR_INVALID_SCH   (int)-2     /*  Invalid scheduler object.   */
#define SCH_ERROR_INVALID_STATE (int)-3     /*  Scheduler/Pool in bad state.    */
#define SCH_ERROR_INTERNAL      (int)-4     /*  Internal error. */
#define SCH_ERROR_POOL_FULL     (int)-5     /*  Pool queue is full. */
#define SCH_ERROR_SIGNAL        (int)-6     /*  Signal failed.  */
#define SCH_ERROR_SYNC_OBJECT   (int)-7     /*  Synchronization object failed.   */

/**
 * Synchronization objects.
 */
typedef void* schSpinLock;           /*	*/
typedef void* schMutexLock;          /*	*/
typedef void* schSemaphore;          /*	*/

/**
 * Scheduler task structure.
 */
typedef int (*schCallback)(struct sch_task_package_t *package);
typedef struct sch_task_package_t {
	/*  Package data.   */
	unsigned int flag;          /*  Package flag.   */
	unsigned int index;         /*  Pool index. */
	/*  User data.  */
	schCallback callback;       /*  Function callback.  */
	unsigned int size;          /*  Size parameter. */
	unsigned int offset;        /*  Offset. */
	void *begin;                /*  Start pointer.  */
	void *end;                  /*  End pointer.    */
	void* puser;                /*  User data.  */
} schTaskPackage;

/**
 * Pool structure.
 */
typedef int (*schUserCallBack)(struct sch_task_pool_t *);
typedef struct sch_task_pool_t {

	/*  Threading.  */
	void *thread;               /*  Thread associated with the pool.    */
	void* schThread;            /*  Scheduler thread.   */

	/*  Thread race condition variables.    */
	void* mutex;                /*	Mutex.	*/
	void* set;                  /*	Thread signal.  */

	/*  Task Queue.  */
	unsigned int size;          /*  */
	unsigned int tail;          /*  */
	unsigned int head;          /*  */
	unsigned int reserved;      /*  Number of allocate task packages.   */
	schTaskPackage *package;    /*  */

	/*  User and init and release functions callbacks.  */
	void *userdata;             /*  */
	schUserCallBack init;       /*  */
	schUserCallBack deinit;     /*  */

	/*  Statistics. */
	int avergeDeque;            /*  Average package dequeue per time unit.  */
	long int dheapPriority;     /*  */

	/*  State and scheduler.    */
	void *sch;                  /*  Scheduler associated with.  */
	unsigned int index;         /*  Affinity index. */
	unsigned int flag;          /*  Pool status flag.   */

} schTaskPool;

/**
 * Task scheduler main struct container.
 */
typedef struct sch_task_schedular_t {
	unsigned int num;       /*  Number of pools.    */
	schTaskPool *pool;      /*  Pools.  */
	unsigned int flag;      /*  State/Status Flags. */
	schTaskPool **dheap;    /*  Priority queue. */
	schSpinLock* spinlock;  /*  Spin lock.  */
	void* set;              /*  Signal listening mask.  */
} schTaskSch;

/**
 * Create scheduler.
 * @param sch object reference.
 * @param cores number of cores allocated. -1 will select
 * all cores.
 * @param flag properties.
 * @param maxPackagesPool number of task on each pool.
 * @return non-negative if successfully releasing.
 */
extern int schCreateTaskPool(schTaskSch *sch, int cores, unsigned int flag, unsigned int maxPackagesPool);

/**
 * Release all resources associated with
 * the scheduler object.
 * @param sch scheduler object.
 * @return non-negative if successfully releasing.
 */
extern int schReleaseTaskSch(schTaskSch *sch);

/**
 * Set initialization callback.
 * @param sch scheduler object.
 * @param callback non-null function pointer.
 */
extern void schSetInitCallBack(schTaskSch *sch, schUserCallBack callBack);

/**
 * Set user deinitialize callback.
 * @param sch scheduler object.
 * @param callback non-null function pointer.
 */
extern void schSetDeInitCallBack(schTaskSch *sch, schUserCallBack callBack);

/**
 * Set the same user data to each pool.
 * @param sch valid schedular object.
 * @param user valid pointer.
 */
extern void schSetSchUserData(schTaskSch* sch, const void* user);

/**
 * Set pool user data.
 * @param index index of the pool from 0.
 * @param user pointer.
 */
extern void schSetPoolUserData(schTaskSch *sch, int index, void *user);

/**
 * Get pool user data.
 * @param sch schedule object.
 * @param index pool index, starting from 0.
 * @return non-null pointer if user pointer exists, NULL otherwise.
 */
extern void *schGetPoolUserData(schTaskSch *sch, int index);

/**
 * Get schedular pool by index.
 * @param sch
 * @param index
 * @return
 */
extern schTaskPool* schGetPool(schTaskSch* sch, int index);

/**
 * Start running task scheduler.
 * This will create the internal thread for each pool
 *
 * @param sch valid schedular object.
 * @return positive if successfully. otherwise failure.
 */
extern int schRunTaskSch(schTaskSch *sch);

/**
 * Terminate the schedular.
 * @param sch valid schedular.
 * @return non-negative if successful.
 */
extern int schTerminateTaskSch(schTaskSch *sch);

/**
 * submit task packet.
 * @param sch valid schedular object.
 * @param package to be sumbit to the pools.
 * @param pPool specific pool queue.
 * @return non-negative if successfully, otherwise failure.
 */
extern int schSubmitTask(schTaskSch *sch, schTaskPackage *package, schTaskPool *pPool);

/**
 * Wait for all pool to finish with all
 * their tasks.
 * @param sch valid schedular object.
 * @return non-negative if successfully.
 */
extern int schWaitTask(schTaskSch *sch);

/**
 * Perform queue enqueue/deqeue that will prevent
 * race condition.
 * @param dequeue non-zero will perform dequeue. otherwise enqueue.
 * @param enqueue enqueue data pointer.
 * @return NULL if enqueue, otherwise data from the dequeue.
 */
extern void* schQueueMutexEnDeQueue(schTaskPool *taskPool, int dequeue, void *enqueue);

/**
 * Lock current task pool in current function
 * @param pool valid pool object.
 */
extern int schPoolLock(schTaskPool *pool);

/**
 * Unlock current task pool in current function
 * @param pool valid pool object.
 */
extern int schPoolUnLock(schTaskPool *pool);

/**
 * Perform priority queue on the pool
 * @param sch non-null scheduler object.
 */
extern void schHeapify(schTaskSch *sch);

/**
 * Get number of CPU cores on the system.
 * @return non-negative number is successfully.
 */
extern int schGetNumCPUCores(void);

/**
 * Get current time.
 * @return non-negative number.
 */
extern long int schGetTime(void);

/**
 * Time resolution per second.
 * @return non-negative number.
 */
extern long int schTimeResolution(void);

/**
 * Create thread.
 * @param affinity core index.
 * @param pfunc function map to the thread.
 * @param userData user data associated with the function.
 * @return non-null if successfully.
 */
extern void *schCreateThread(int affinity, void *pfunc, void *userData);

/**
 * Release thread resources.
 * @param thread valid thread.
 * @return non-negative if successfully released, otherwise a failure.
 */
extern int schDeleteThread(void *thread);

/**
 * Wait in till thread is finished with
 * the callback entry function.
 * @param thread valid thread.
 * @return non-negative if successfully.
 */
extern int schWaitThread(void* thread);

/**
 * Set thread name.
 * @param thread valid thread.
 * @param name non-null terminated string.
 * @return success status.
 */
extern int schSetThreadName(void *thread, const char *name);

/**
 * Get current thread pointer object.
 * @return non-null thread if successfully.
 */
extern void *schCurrentThread(void);

/**
 * Raise signal to specified thread.
 * @param thread valid thread object.
 * @param signal valid signal.
 * @return non-negative if successfully.
 */
extern int schRaiseThreadSignal(void* thread, int signal);

/**
 * Allocate signal object.
 * @return non-null if successfully.
 */
extern void* schCreateSignal(void);

/**
 * Release signal resources.
 * @param signal valid signal pointer.
 * @return
 */
extern int schDeleteSignal(void* signal);

/**
 * Get the base signal number that
 * are valid for sending signal between threads and
 * that will not conflicts with the kernel specified
 * signals.
 * @return non-negative number.
 */
extern int schBaseSignal(void);

/**
 * Wait in till signal has been issued.
 * @param sig signal object.
 * @return signal received.
 */
extern int schSignalWait(void* sig);

/**
 * Wait in till a signal has been issued in
 * the timeout time frame.
 * @param sig signal object.
 * @param time in nano seconds for the timeout.
 * @return received.
 */
extern int schSignalWaitTimeOut(void* sig, long int nano);

/**
 * Set thread signal mask. Mask what signal
 * to listen and how to 
 *
 * @param signal object.
 * @param nr number of signals.
 * @param signals array of valid signals.
 * @return non-negative if successfully.
 */
extern int schSetSignalThreadMask(void* set, int nr, const int* signals);

/**
 * Create mutex pointer.
 * @param mutex non-null pointer to mutex pointer.
 * @return non-zero if successfully.
 */
extern int schCreateMutex(schMutexLock **mutex);

/**
 * Create spinlock synchronize primitive
 * object.
 * @param spinlock valid pointer.
 * @return non-negative if successfully.
 */
extern int schCreateSpinLock(schSpinLock** spinlock);

/**
 * Create semaphore object.
 * @param pSemaphore valid pointer.
 * @return non-negative if successfully.
 */
extern int schCreateSemaphore(schSemaphore** pSemaphore);

/**
 * Release resources associated with the mutex object.
 * @param mutex valid mutex object pointer.
 * @return non-negative if successfully.
 */
extern int schDeleteMutex(schMutexLock *mutex);

/**
 * Release spinlock resources.
 * @param spinlock valid spinlock pointer.
 * @return non-negative if successfully.
 */
extern int schDeleteSpinLock(schSpinLock* spinlock);

/**
 * Delete semaphore.
 * @param pSemaphore valid pointer.
 * @return non-negative if successfully.
 */
extern int schDeleteSemaphore(schSemaphore* pSemaphore);

/**
 * Lock spinlock.
 * @param spinlock valid spinlock pointer.
 * @return non-negative if successfully.
 */
extern int schLockSpinLock(schSpinLock *spinlock);

/**
 * Unlock spinlock.
 * @param spinlock valid spinlock pointer.
 * @return non-negative if successfully.
 */
extern int schUnlockSpinLock(schSpinLock *spinlock);

/**
 * Pool thread execution environment.
 * @param handle user specified data.
 * @return NULL
 */
extern void *schPoolExecutor(void *handle);

/**
 * Get error code message.
 * @param errMsg zero or negative number.
 * @return non-null terminated string.
 */
extern const char* schErrorMsg(int errMsg);

#ifdef __cplusplus
}
#endif

#endif
