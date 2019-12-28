#define _GNU_SOURCE

#include"taskSch.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#include<pthread.h>
#include<semaphore.h>
#include<signal.h>
#include<unistd.h>

schThread *schCreateThread(int affinity, schFunc *pfunc, void *userData) {

	pthread_t t0;
	pthread_attr_t attr;
	int mpid;
	cpu_set_t cpus;

	/*  Set CPU affinity mapping.   */
	CPU_ZERO(&cpus);
	CPU_SET(affinity, &cpus);

	pthread_attr_init(&attr);

	/*  Create affinity mapping.    */
	if (affinity >= 0) {
		/*  Set CPU affinity thread attribute.  */
		if (pthread_attr_setaffinity_np(&attr, sizeof(cpus), &cpus) != 0) {
			fprintf(stderr, "failed creating attribute affinity: %s.\n", strerror(errno));
			return NULL;
		}
	}

	/*  Create thread. */
	if ((mpid = pthread_create(&t0, &attr, pfunc, userData)) != 0) {
		fprintf(stderr, "failed creating thread: %s.\n", strerror(errno));
		return NULL;
	}

	/*  Release attribute object.   */
	pthread_attr_destroy(&attr);
	return (schThread *) t0;
}


static int pthread_error_code2sch_error_code(int error) {
	switch (error) {
		case ESRCH:
		case 0:
			return SCH_OK;
		case EBUSY:
			return SCH_ERROR_BUSY;
		case EINVAL:
			return SCH_ERROR_INVALID_ARG;
		case EPERM:
			return SCH_ERROR_PERMISSION_DENIED;
		case EAGAIN:
			return SCH_ERROR_LACK_OF_RESOURCES;
		case ENOMEM:
			return SCH_ERROR_NOMEM;
		case EINTR:
		case EDEADLK:
		case ENOSYS:
		default:
			return SCH_ERROR_UNKNOWN;
	}
}

int schDeleteThread(schThread *thread) {
	return pthread_error_code2sch_error_code(pthread_detach((pthread_t) thread));
}

int schWaitThread(schThread *thread) {
	return pthread_error_code2sch_error_code(pthread_join((pthread_t) thread, NULL));
}

schThread *schCurrentThread(void) {
	return (schThread *) pthread_self();
}

int schCreateMutex(schMutex **mutex) {
	*mutex = malloc(sizeof(pthread_mutex_t));
	assert(*mutex);

	return pthread_error_code2sch_error_code(pthread_mutex_init((pthread_mutex_t *) *mutex, NULL));//== 0;
}

int schCreateSpinLock(schSpinLock **spinlock) {
	*spinlock = malloc(sizeof(pthread_spinlock_t));
	assert(*spinlock);

	return pthread_error_code2sch_error_code(pthread_spin_init(*spinlock, 0));
}

int schCreateSemaphore(schSemaphore **pSemaphore) {
	sem_t *sem = (sem_t *) malloc(sizeof(sem_t));
	*pSemaphore = (schSemaphore *) sem;
	assert(sem);

	if(sem_init(sem, 0, 0))
		return SCH_OK;
	else
		return pthread_error_code2sch_error_code(errno);
}

int schDeleteMutex(schMutex *mutex) {
	int status = pthread_error_code2sch_error_code(pthread_mutex_destroy((pthread_mutex_t *) mutex));
	free(mutex);
	return status;
}

int schDeleteSpinLock(schSpinLock *spinlock) {
	int status = pthread_error_code2sch_error_code(pthread_spin_destroy(spinlock));
	free(spinlock);
	return status;
}

int schDeleteSemaphore(schSemaphore *pSemaphore) {
	sem_t *sem = pSemaphore;
	int status = pthread_error_code2sch_error_code(sem_destroy(sem));
	free(pSemaphore);
	return status;
}

int schLockSpinLock(schSpinLock *spinlock) {
	return pthread_error_code2sch_error_code(pthread_spin_lock(spinlock));
}

int schTryLockSpinLock(schSpinLock *spinLock) {
	return pthread_error_code2sch_error_code(pthread_spin_trylock(spinLock));
}

int schUnlockSpinLock(schSpinLock *spinlock) {
	return pthread_error_code2sch_error_code(pthread_spin_unlock(spinlock));//== 0 ? SCH_OK : SCH_ERROR_UNKNOWN;
}

int schGetNumCPUCores(void) {
#ifdef __unix__
	return (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__IRIX__)
	return  sysconf(_SC_NPROC_ONLN);
#elif defined(_SC_NPROCESSORS_ONLN)
	/* number of processors online (SVR4.0MP compliant machines) */
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROCESSORS_CONF)
	/* number of processors configured (SVR4.0MP compliant machines) */
	return sysconf(_SC_NPROCESSORS_CONF);
#endif
}

#define MAX_THREAD_NAME 16

int schSetThreadName(schThread *thread, const char *threadName) {
	int status = pthread_error_code2sch_error_code(pthread_getname_np((pthread_t) thread, threadName, MAX_THREAD_NAME));
	return status;
}

int schRaiseThreadSignal(schThread *thread, int signal) {
	return pthread_error_code2sch_error_code(
			pthread_kill((pthread_t) thread, signal));
}

schSignalSet *schCreateSignal(void) {
	sigset_t *sig = (sigset_t *) malloc(sizeof(sigset_t));
	assert(sig);
	return sig;
}

int schDeleteSignal(schSignalSet *signal) {
	free(signal);
	return SCH_OK;
}

int schBaseSignal(void) {
	return SIGUSR1;
}

int schSignalWait(schSignalSet *sig) {
	int signal;
	if (sigwait(sig, &signal) == 0)
		return signal;
	else
		return -1;
}

int schSignalWaitTimeOut(schSignalSet *sig, long int nano) {
	siginfo_t info;
	struct timespec spec;

	spec.tv_sec = nano / 1000000000;
	spec.tv_nsec = nano % 1000000000;

	int status = sigtimedwait(sig, &info, &spec);
	return info.si_signo;
}


int schSetSignalThreadMask(schSignalSet *set, int nr, const int *signals) {

	int i;
	int err;
	assert(sigemptyset(set) == 0);

	/*  Set each signal masks.  */
	for (i = 0; i < nr; i++) {
		err = sigaddset(set, signals[i]);

		/*  */
		if (err != 0) {
			fprintf(stderr, "failed mapping thread: %s\n", strerror(err));

		}
	}

	/*  Associated thread with signal mask. */
	err = pthread_sigmask(SIG_BLOCK, set, NULL);
	if (err != 0) {
		fprintf(stderr, "failed mapping thread: %s\n", strerror(err));
		return -1;
	}
	return pthread_error_code2sch_error_code(err);
}


int schMutexLock(schMutex *mutex) {
	int error = pthread_mutex_lock(mutex);
	return pthread_error_code2sch_error_code(error);
}

int schMutexTryLock(schMutex *mutex, long int timeout) {
	struct timespec spec;

	spec.tv_sec = timeout / 1000000000;
	spec.tv_nsec = timeout % 1000000000;

	return pthread_error_code2sch_error_code(pthread_mutex_timedlock(mutex, &spec));
}

int schMutexUnLock(schMutex *mutex) {
	int error = pthread_mutex_unlock(mutex);
	if (error == 0)
		return SCH_OK;
	return pthread_error_code2sch_error_code(error);
}

int schSemaphoreWait(schSemaphore *pSemaphore) {
	if (sem_wait(pSemaphore) == 0)
		return SCH_OK;
	else
		pthread_error_code2sch_error_code(errno);
}

int schSemaphoreTimedWait(schSemaphore *pSemaphore, long int timeout) {
	struct timespec spec;

	spec.tv_sec = timeout / 1000000000;
	spec.tv_nsec = timeout % 1000000000;

	if (sem_timedwait(pSemaphore, &spec) == 0)
		return SCH_OK;
	else
		pthread_error_code2sch_error_code(errno);
}

int schSemaphorePost(schSemaphore *pSemaphore) {
	if (sem_post((sem_t *) pSemaphore) == 0)
		return SCH_OK;
	else
		pthread_error_code2sch_error_code(errno);
}

int schSemaphoreValue(schSemaphore *pSemaphore, int *value) {
	if (sem_getvalue((sem_t *) pSemaphore, value) == 0)
		return SCH_OK;
	else
		return pthread_error_code2sch_error_code(errno);
}

int schPoolLock(schTaskPool *pool) {
	return schMutexLock(pool->mutex);
}

int schPoolUnLock(schTaskPool *pool) {
	return schMutexUnLock(pool->mutex);
}