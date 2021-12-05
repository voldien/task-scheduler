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
#ifndef _CORE_TASK_SCH_H_
#define _CORE_TASK_SCH_H_ 1

/**
 * @file taskSch.h
 * @ingroup libtasksch
 * Main libtasksch public API header
 */

/**
 * @defgroup libtasksch Task Scheduler
 * @brief
 *
 * The main task scheduler uses a pool task architecture. That means that it create
 * a set of task pools. Each pool can be assigned with a task that will be scheduled
 * by the scheduler.
 * @{
 *
 * @defgroup ltasksch_error Error Codes
 * @{
 * @}
 *
 * @defgroup ltasksch_threads Thread
 * @{
 * @}
 *
 * @defgroup ltasksch_syncs Thread Synchronization
 * @{
 * @}
 * @}
 *
 *
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <atomic>
using namespace std;
#else
#include <stdatomic.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Macro for adding library
 * support.
 */
#ifndef TASH_SCH_EXTERN
#ifndef TASH_SCH_STATIC
#ifdef _WIN32
#define TASH_SCH_EXTERN __declspec(dllimport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define TASH_SCH_EXTERN __attribute__((visibility("default")))
#else
#define TASH_SCH_EXTERN
#endif
#else
#define TASH_SCH_EXTERN
#endif
#endif

/**
 * Task scheduler option flags.
 */
#define SCH_FLAG_NO_AFM 0x80000000 /*  Disable affinity mapping.    */

/**
 * Task scheduler internal flags.
 *
 */
#define SCH_FLAG_INIT 0x1	 /*  Scheduler has been initialized.    */
#define SCH_FLAG_RUNNING 0x2 /*  Scheduler is in running mode.   */
#define SCH_FLAG_IDLE 0x4

/**
 * Pool status flags.
 */
#define SCH_POOL_TERMINATE 0x1 /*  Pool has been terminated.   */
#define SCH_POOL_RUNNING 0x2   /*  Pool is in running mode.    */
#define SCH_POOL_SLEEP 0x4	   /*  Pool is in sleep mode.  */

/**
 * Task scheduler signals.
 */
#define SCH_SIGNAL_IDLE (schBaseSignal() + 0)	  /*  */
#define SCH_SIGNAL_RUNNING (schBaseSignal() + 1)  /*  */
#define SCH_SIGNAL_DONE (schBaseSignal() + 2)	  /*  */
#define SCH_SIGNAL_CONTINUE (schBaseSignal() + 3) /*  */
#define SCH_SIGNAL_QUIT (schBaseSignal() + 4)	  /*  */
#define SCH_SIGNAL_STOP SIGSTOP

/**
 * @addtogroup ltasksch_error
 * @ingroup libtasksch
 * @brief Error Codes Library Specific Error codes.
 * @{
 */
/**
 * @brief
 *
 */
enum SchErrCode {
	SCH_OK = 1,							 /*  No error.   */
	SCH_ERROR_UNKNOWN = 0,				 /*  Error unknown.   */
	SCH_ERROR_INVALID_ARG = -1,			 /*  Invalid argument.   */
	SCH_ERROR_INVALID_SCH = (-2),		 /*  Invalid scheduler object.   */
	SCH_ERROR_INVALID_STATE = (-3),		 /*  Scheduler/Pool in bad state.    */
	SCH_ERROR_INTERNAL = (-4),			 /*  Internal error. */
	SCH_ERROR_POOL_FULL = (-5),			 /*  Pool queue is full. */
	SCH_ERROR_SIGNAL = (-6),			 /*  Signal failed.  */
	SCH_ERROR_SYNC_OBJECT = (-7),		 /*  Synchronization object failed.   */
	SCH_ERROR_TIMEOUT = (-8),			 /*  Timeout.    */
	SCH_ERROR_BUSY = (-9),				 /*  Busy error. */
	SCH_ERROR_NOMEM = (-10),			 /*  No Memory.  */
	SCH_ERROR_LACK_OF_RESOURCES = (-11), /*  There system is lacking resources.  */
	SCH_ERROR_PERMISSION_DENIED = (-12), /*  Permission denied of the operation. */
};
/**
 * @}
 */

/**
 * Task function callback type.
 */
typedef void *(*schFunc)(void *pdata); /*  */

/**
 * @addtogroup ltasksch_syncs
 * @{
 *
 */
typedef void schSpinLock;	 /*	Spinlock sync object.   */
typedef void schMutex;		 /*	Mutex (mutual exclusion) sync object. */
typedef void schSemaphore;	 /*	Semaphore sync object.  */
typedef void schRWLock;		 /*  Read/Write lock.	*/
typedef void schConditional; /*  Conditional lock.	*/
typedef void schBarrier;	 /*  Memory Barrier.	*/
/**
 * @}
 */

/**
 * @defgroup ltascsch_thread_objects Thread Primitives
 * @ingroup libtasksch
 * @brief
 * @{
 */
typedef void schThread;	   /*  Thread object.  */
typedef void schSignalSet; /*  Signal set object.  */
/**
 * @}
 */

typedef struct sch_task_scheduler_t schTaskSch;
typedef struct sch_task_pool_t schTaskPool;
typedef struct sch_task_package_t schTaskPackage;

typedef int (*schUserCallBack)(struct sch_task_pool_t *);
typedef int (*schCallback)(schTaskPackage *package);

// typedef struct sch_task_package_t schTaskPackage;
/**
 * Scheduler task structure.
 */
typedef struct sch_task_package_t {
	/*  Package data.   */
	/**
	 *
	 */
	atomic_uint flag; /*  Package flag.   */
	/**
	 * Pool index it was executed from.
	 */
	unsigned int index;
	/**
	 * Callback function that the scheduler will
	 * execute.
	 */
	schCallback callback;
	// TODO determine to add long support.
	/**
	 * Size parameter.
	 *
	 */
	size_t size;
	/**
	 * Offset
	 *
	 */
	size_t offset;
	/**
	 * Start pointer.
	 *
	 */
	void *begin;
	/**
	 * End pointer.
	 *
	 */
	void *end;
	/**
	 * User data.
	 *
	 */
	void *puser;
} schTaskPackage;

/**
 * @defgroup ltascsch_core Core functions
 * @ingroup libtasksch
 * @section Info
 *
 * @code
 * @endcode
 *
 * @{
 */

/**
 * @brief Allocate task scheduler object. It will make sure
 * that for any version of the library, that the size of the internal
 * structure are of correct size.
 *
 * @since 0.2.0
 * @param pSch valid pointer object.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schAllocateTaskPool(schTaskSch **pSch);

/**
 * Initilize task scheduler internal data structure.
 *
 * @see schAllocateTaskPool
 *
 * @param sch object reference.
 * @param cores number of pool that will be allocated. -1 will select
 * number of pools that matches the number of cores.
 * @param flag properties.
 * @param maxPackagesPool Sets the max number of task on each pool can have in the queue
 * at any given moment.
 * @return non-negative if successfully releasing. @see
 * @since 1.0.0
 */
extern TASH_SCH_EXTERN int schCreateTaskPool(schTaskSch *sch, int cores, unsigned int flag,
											 unsigned int maxPackagesPool);

/**
 * Release all resources associated with
 * the scheduler object.
 *
 * @param sch scheduler object.
 * @return non-negative if successfully releasing.
 */
extern TASH_SCH_EXTERN int schReleaseTaskSch(schTaskSch *sch);

/**
 * Set initialization callback that will be invoked when the scheduler starts.
 *
 * @param sch scheduler object.
 * @param callBack non-null function pointer.
 */
extern TASH_SCH_EXTERN void schSetInitCallBack(schTaskSch *sch, schUserCallBack callBack);

/**
 * Set user deinitialize callback that will be invoked when the scheduler gets terminated.
 *
 * @param sch scheduler object.
 * @param callBack non-null function pointer.
 */
extern TASH_SCH_EXTERN void schSetDeInitCallBack(schTaskSch *sch, schUserCallBack callBack);

/**
 * Assign user data associated with the scheduler object.
 * @param sch valid scheduler object.
 * @param user valid pointer.
 */
extern TASH_SCH_EXTERN void schSetSchUserData(schTaskSch *sch, const void *user);

/**
 * Assign user data associated with the scheduler objects pools.
 * @param sch valid scheduler object.
 * @param index valid pool index, where \f$ index \in [0, nrPools -1] \f$.
 * @param user valid pointer.
 * @see schGetPoolUserData
 */
extern TASH_SCH_EXTERN void schSetPoolUserData(schTaskSch *sch, int index, const void *user);

/**
 * Get pool user data.
 * @param sch schedule object.
 * @param index valid pool index, where \f$ index \in [0, nrPools -1] \f$.
 * @return non-null pointer if user pointer exists, NULL otherwise.
 * @see schSetPoolUserData
 */
extern TASH_SCH_EXTERN void *schGetPoolUserData(schTaskSch *sch, int index);

/**
 * Get scheduler pool by index.
 * @param sch
 * @param index
 * @return
 */
extern TASH_SCH_EXTERN schTaskPool *schGetPool(schTaskSch *sch, int index);

/**
 * @}
 */

/**
 * @defgroup ltascsch_func   Scheduler functionalities.
 * @ingroup libtasksch
 *
 *
 *
 * @{
 */

/**
 * Start running task scheduler.
 * This will create the internal thread for each pool followed by the startup sequence.
 *
 * @see schSetInitCallBack for setup a custom callback before starting the pool.
 *
 * @param sch valid scheduler object.
 * @return positive if successfully. otherwise failure.
 */
extern TASH_SCH_EXTERN int schRunTaskSch(schTaskSch *sch);

/**
 * Stop all current tasks.
 * @param sch valid scheduler object.
 * @param timeout_nanoseconds how long it w
 * @return non-negative if succesfull.
 */
extern TASH_SCH_EXTERN int schStopTaskSch(schTaskSch *sch, long int timeout_nanoseconds);

/**
 * Terminate the scheduler.
 *
 * @param sch valid scheduler object.
 * @return non-negative if succesfull.
 */
extern TASH_SCH_EXTERN int schTerminateTaskSch(schTaskSch *sch); // TODO give timeout option perhaps.

/**
 * submit a task packet, it will be assigned accordingly to the priority queue, unless
 * user specifies which pool it will be foced onto.
 *
 * @remark When overriding the pool that the task will be assigned that result in both performance lost
 * and failure from being queue being full.
 *
 * @param sch valid scheduler object.
 * @param package valid task package, that will be sumbitted to the pools.
 * @param pPool specific pool queue.
 * @return non-negative if successfull, otherwise failure.
 */
extern TASH_SCH_EXTERN int schSubmitTask(schTaskSch *sch, schTaskPackage *package, schTaskPool *pPool);

/**
 * Remove all tasks on all of the pools on the scheduler. But if the pool argument is specified
 * it will clear only a single specific pool.
 *
 * @param sch
 * @param pool
 * @return
 */
extern TASH_SCH_EXTERN int schClearTask(schTaskSch *sch, schTaskPool *pool);

/**
 * @brief
 *
 * @see schClearTask will be executed similar.
 * @param sch
 * @return non-negative if successfull, otherwise a errorcode.
 */
extern TASH_SCH_EXTERN int schClearAllTask(schTaskSch *sch);

/**
 * Wait for all pool to finish with all their tasks.
 *
 * @param sch valid scheduler object.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schWaitTask(schTaskSch *sch);

/**
 * Wait for all pool to finish with all
 * their tasks.
 * @param sch valid scheduler object.
 * @param wait time in nano seconds.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schWaitTaskWait(schTaskSch *sch, long wait);

/**
 * Lock current task pool in current function
 * @param pool valid pool object.
 */
extern TASH_SCH_EXTERN int schPoolLock(schTaskPool *pool);

/**
 * Unlock current task pool in current function
 * @param pool valid pool object.
 */
extern TASH_SCH_EXTERN int schPoolMutexUnLock(schTaskPool *pool);

/**
 * @}
 */

/**
 * @addtogroup ltasksch_threads
 *
 * @{
 */

/**
 * @brief Create new thread object with a custom callback entrypoint
 *
 *
 *
 * @param affinity core index.
 * @param pfunc function map to the thread.
 * @param userData user data associated with the function.
 * @return non-null if successfully. If NULL,
 */
extern TASH_SCH_EXTERN schThread *schCreateThread(int affinity, schFunc pfunc, void *userData);

/**
 * @brief Delete thread
 * This will cause the system to release the thread resources.
 *
 * @param thread valid thread.
 * @return non-negative if successfully released, otherwise a failure.
 */
extern TASH_SCH_EXTERN int schDeleteThread(schThread *thread);

/**
 * Wait in till thread is finished with
 * the callback entry function.
 * @param thread valid thread.
 * @param retval
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schWaitThread(schThread *thread, void **retval);

/**
 * Set thread name.
 * @param thread valid thread.
 * @param name non-null terminated string.
 * @return success status.
 */
extern TASH_SCH_EXTERN int schSetThreadName(schThread *thread, const char *name);

/**
 * Get current thread pointer object.
 *
 * @return non-null thread if successfully.
 */
extern TASH_SCH_EXTERN schThread *schCurrentThread(void);

/**
 * Raise a specific signal to a specified thread.
 *
 * @param thread valid thread object.
 * @param signal valid signal.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schRaiseThreadSignal(schThread *thread, int signal);

/**
 * @}
 */

/**
 * @addtogroup ltasksch_syncs
 * @brief Syncronization Functions
 * @{
 */

/**
 * Allocate signal object.
 * @return non-null if successfully.
 */
extern TASH_SCH_EXTERN schSignalSet *schCreateSignal(void);

/**
 * Release signal resources.
 * @param signal valid signal pointer.
 * @return
 */
extern TASH_SCH_EXTERN int schDeleteSignal(schSignalSet *signal);

/**
 * Get the base signal number that
 * are valid for sending signal between threads and
 * that will not conflicts with the kernel specified
 * signals.
 *
 * @return non-negative number.
 */
extern TASH_SCH_EXTERN int schBaseSignal(void);

/**
 * @brief
 * Wait in till signal has been issued.
 *
 * @see
 * @param sig signal object.
 * @return signal received.
 */
extern TASH_SCH_EXTERN int schSignalWait(schSignalSet *sig);

/**
 * Wait in till a signal has been issued in
 * the timeout time frame.
 *
 * @param sig signal object.
 * @param nanoseconds in nano seconds for the timeout.
 * @return signal.
 */
extern TASH_SCH_EXTERN int schSignalWaitTimeOut(schSignalSet *sig, long int nanoseconds);

/**
 * Set thread signal mask. Mask what signal
 * to listen and how to
 *
 * @param set object.
 * @param nr number of signals.
 * @param signals array of valid signals.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schSetSignalThreadMask(schSignalSet *set, int nr, const int *signals);

/**
 * Create mutex pointer.
 * @param mutex non-null pointer to mutex pointer.
 * @return non-zero if successfully.
 */
extern TASH_SCH_EXTERN int schCreateMutex(schMutex **mutex);

/**
 * @brief Creates a spinlock synchronization primitive
 * object.
 *
 * The Synchronization primitive works by using a thight
 * for loop and check if still locked or if has been unlocked and can
 * proceeded.
 *
 * Use this synchronization primitive when the lock is suspected to be very
 * short or if to prevent the kernel from switching the process directly.
 *
 * @see schLockSpinLock
 * @see schTryLockSpinLock
 * @see schUnlockSpinLock
 * @param spinlock valid pointer.
 * @return non-negative if successfull.
 */
extern TASH_SCH_EXTERN int schCreateSpinLock(schSpinLock **spinlock);

/**
 * Create semaphore object.
 * @param pSemaphore valid pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schCreateSemaphore(schSemaphore **pSemaphore);

/**
 * @brief
 *
 * @param pBarrier
 * @return
 */
extern TASH_SCH_EXTERN int schCreateBarrier(schBarrier **pBarrier);
/**
 * @brief
 *
 * @param pBarrier
 * @param count
 * @return
 */
extern TASH_SCH_EXTERN int schInitBarrier(schBarrier *pBarrier, int count);

/**
 * @brief
 *
 * @param barrier
 * @return
 */
extern TASH_SCH_EXTERN int schDeleteBarrier(schBarrier *barrier);
/**
 * @brief
 *
 * @param barrier
 * @return
 */
extern TASH_SCH_EXTERN int schWaitBarrier(schBarrier *barrier);

/**
 * @brief
 *
 * @param pCondVariable
 * @return
 */
extern TASH_SCH_EXTERN int schCreateConditional(schConditional **pCondVariable);

/**
 * @brief
 *
 * @param conditional
 * @return
 */
extern TASH_SCH_EXTERN int schDeleteConditional(schConditional *conditional);

/**
 * @brief
 *
 * @param conditional
 * @param mutex
 * @return
 */
extern TASH_SCH_EXTERN int schConditionalWait(schConditional *conditional, schMutex *mutex);

/**
 * @brief
 *
 * @param conditional
 * @return
 */
extern TASH_SCH_EXTERN int schConditionalSignal(schConditional *conditional);

/**
 * @brief
 *
 * @param pRwLock
 * @return
 */
extern TASH_SCH_EXTERN int schCreateRWLock(schRWLock **pRwLock);

/**
 * @brief
 *
 * @param rwLock
 * @return
 */
extern TASH_SCH_EXTERN int schDeleteRWLock(schRWLock *rwLock);

/**
 * @brief
 *
 * @param rwLock
 * @return
 */
extern TASH_SCH_EXTERN int schRWLockRead(schRWLock *rwLock);

/**
 * @brief
 *
 * @param rwLock
 * @return
 */
extern TASH_SCH_EXTERN int schRWLockWrite(schRWLock *rwLock);

/**
 * @brief
 *
 * @param rwLock
 * @return
 */
extern TASH_SCH_EXTERN int schRWLocUnLock(schRWLock *rwLock);

/**
 * Release resources associated with the mutex object.
 * @param mutex valid mutex object pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schDeleteMutex(schMutex *mutex);

/**
 * Release spinlock resources.
 * @param spinlock valid spinlock pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schDeleteSpinLock(schSpinLock *spinlock);

/**
 * Delete semaphore.
 * @param pSemaphore valid pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schDeleteSemaphore(schSemaphore *pSemaphore);

/**
 * Lock mutex and wait initill it has been unlocked for the
 * thread to use the mutex.
 * @param mutexLock valid mutex pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schMutexLock(schMutex *mutexLock);

/**
 * Attempt to lock the mutex. If the wait time exceeds the timeout
 * it will return with the status of timeout.
 * @param mutex valid mutex pointer object.
 * @param timeout number of nanoseconds that it will wait.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schMutexTryLock(schMutex *mutex, long int timeout);

/**
 * Unlock mutex.
 * @param mutexLock valid mutex pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schMutexUnLock(schMutex *mutexLock);

/**
 * If the counter is it will block and wait for the
 * schSemaphorePost function to be wait. Otherwise it
 * will continue execution while blocking any other thread
 * passing the wait function in till the schSemaphorePost.
 * function is invoked.
 * Wait in till the semaphore has been unlocked.
 *
 * @param pSemaphore valid semaphore object.
 * @return non-negative if successfull.
 */
extern TASH_SCH_EXTERN int schSemaphoreWait(schSemaphore *pSemaphore);

/**
 * Similar to schSemaphoreWait execpet it won't be blocking if
 * blocked by other semaphore and will return status that it is
 * timed out.
 * @param semaphore valid semaphore object.
 * @return non-negative if successfully, SCH_ERROR_TIMEOUT
 */
extern TASH_SCH_EXTERN int schSemaphoreTryWait(schSemaphore *semaphore);

/**
 * Wait for the semaphore has been unlocked for
 * a explicit duration of time in nanoseconds.
 * @param pSemaphore valid semaphore object.
 * @param timeout non-negative time in nanoseconds.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schSemaphoreTimedWait(schSemaphore *pSemaphore, long int timeout);

/**
 * Function will decrease the counter and let next
 * first waiting semaphore continue while looking the
 * remaining waiting semaphore.
 * @param pSemaphore valid semaphore object.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schSemaphorePost(schSemaphore *pSemaphore);

/**
 * Get current value of the semaphore.
 * @param pSemaphore valid semaphore object.
 * @param value
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schSemaphoreValue(schSemaphore *pSemaphore, int *value);

/**
 * Lock spinlock.
 * @param spinlock valid spinlock pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schLockSpinLock(schSpinLock *spinlock);

/**
 * Attempt to lock the spinlock. If failed it will return directly
 * rather than wait.
 * @param spinLock valid spinlock pointer object.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schTryLockSpinLock(schSpinLock *spinLock);

/**
 * Unlock spinlock.
 * @param spinlock valid spinlock pointer.
 * @return non-negative if successfully.
 */
extern TASH_SCH_EXTERN int schUnlockSpinLock(schSpinLock *spinlock);

/**
 * @}
 */

/**
 * @addtogroup ltasksch_error
 *
 * @{
 */

/**
 * Get error code human readable error-message.
 *
 * @see SchErrCode
 * @remark Do not call free or delete on the return character pointer.
 * This is because the error message are stored in read only memory in the library.
 *
 * @param errMsg zero or negative number.
 * @return non-null terminated string.
 */
extern TASH_SCH_EXTERN const char *schErrorMsg(int errMsg);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
