#define _GNU_SOURCE

#include"taskSch.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#include<pthread.h>
#include<signal.h>
#include<unistd.h>

void *schCreateThread(int affinity, void *pfunc, void *userData) {

	pthread_t t0;
	pthread_attr_t attr;
	int mpid;
	cpu_set_t cpus;

	/*  Set CPU affinity mapping.   */
	CPU_ZERO(&cpus);
	CPU_SET(affinity, &cpus);

	pthread_attr_init(&attr);

	/*  */
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

	/*  */
	pthread_attr_destroy(&attr);
	return (void *) t0;
}

int schDeleteThread(void *thread) {
	if (pthread_detach(thread) == -1) {
		fprintf(stderr, strerror(errno));
		return SCH_ERROR_INTERNAL;
	}
	return SCH_OK;
}

int schWaitThread(void *thread) {
	if (pthread_join((pthread_t) thread, NULL) == -1) {
		fprintf(stderr, strerror(errno));
		return 0;
	}
	return 1;
}

void *schCurrentThread() {
	return (void *) pthread_self();
}

int schCreateMutex(void **mutex) {
	*mutex = malloc(sizeof(pthread_mutex_t));
	assert(*mutex);

	return pthread_mutex_init((pthread_mutex_t *) *mutex, NULL) == 0;
}

int schDeleteMutex(void *mutex) {
	pthread_mutex_destroy((pthread_mutex_t *) mutex);
	free(mutex);
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

int schSetThreadName(void *thread, const char *threadName) {
	int status = pthread_getname_np((pthread_t) thread, threadName, MAX_THREAD_NAME);
	return status == 0;
}

int schRaiseThreadSignal(void *thread, int signal) {
	return pthread_kill(thread, signal) == 0;
}

void *schCreateSignal(void) {
	sigset_t *sig = (sigset_t *) malloc(sizeof(sigset_t));
	assert(sig);
	return sig;
}

int schBaseSignal(void) {
	return SIGUSR1;
}

int schSignalWait(void *sig) {
	int signal;
	sigwait(sig, &signal);
	return signal;
}

int schSignalWaitTimeOut(void *sig, long int time) {
	siginfo_t info;
	struct timespec spec;

	spec.tv_sec = 0;
	spec.tv_nsec = time;

	sigtimedwait(sig, &info, &spec);
	return info.si_signo;
}


int schSetSignalThreadMask(void *set, int nr, const int *signals) {

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
	return 1;
}

void schPoolLock(schTaskPool *pool) {
	pthread_mutex_lock(pool->mutex);
}

void schPoolUnLock(schTaskPool *pool) {
	pthread_mutex_unlock(pool->mutex);
}