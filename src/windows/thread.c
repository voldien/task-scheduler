#include "taskSch.h"

#include "taskSch.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <strsafe.h>
#include <tchar.h>
#include <windows.h>
#include <winerror.h>

static int window_error_code2sch_error_code(DWORD error) {}

schThread *schCreateThread(int affinity, schFunc pfunc, void *userData) {

	DWORD dwThreadIdArray;
	schThread *thread = CreateThread(NULL,	   // default security attributes
									 0,		   // use default stack size
									 pfunc,	   // thread function name
									 userData, // argument to thread function
									 0,		   // use default creation flags
									 NULL);	   // returns the thread identifier
}

int schDeleteThread(schThread *thread) { ExitThread(thread); }

int schWaitThread(schThread *thread, void **retval) {
	return pthread_error_code2sch_error_code(pthread_join((pthread_t)thread, retval));
}

schThread *schCurrentThread(void) { return (schThread *)GetCurrentThread(); }

int schCreateMutex(schMutex **mutex) {
	// HANDLE mutex = CreateMutexA()
}

int schCreateSpinLock(schSpinLock **spinlock) {}

int schCreateSemaphore(schSemaphore **pSemaphore) {
	// CreateSemaphoreA()
}

int schCreateBarrier(schBarrier **pBarrier) {
	*pBarrier = (schBarrier *)malloc(sizeof(pthread_barrier_t));
	memset(*pBarrier, 0, sizeof(pthread_barrier_t));
	// pthread_barrier_init()
	return SCH_OK;
}

int schInitBarrier(schBarrier *pBarrier, int count) {}

int schDeleteBarrier(schBarrier *barrier) {}

int schWaitBarrier(schBarrier *barrier) {}

int schCreateConditional(schConditional **pCondVariable) {}

int schDeleteConditional(schConditional *conditional) {}

int schConditionalWait(schConditional *conditional, schMutex *mutex) {}

int schConditionalSignal(schConditional *conditional) {}

int schCreateRWLock(schRWLock **pRwLock) {}

int schDeleteRWLock(schRWLock *rwLock) {}

int schRWLockRead(schRWLock *rwLock) {}
int schRWLockWrite(schRWLock *rwLock) {}
int schRWLocUnLock(schRWLock *rwLock) {}

int schDeleteMutex(schMutex *mutex) {}

int schDeleteSpinLock(schSpinLock *spinlock) {}

int schDeleteSemaphore(schSemaphore *pSemaphore) { CloseHandle(pSemaphore); }

int schLockSpinLock(schSpinLock *spinlock) {}

int schTryLockSpinLock(schSpinLock *spinLock) {}

int schUnlockSpinLock(schSpinLock *spinlock) {}

int schGetNumCPUCores(void) {}

int schSetThreadName(schThread *thread, const char *threadName) {}

int schRaiseThreadSignal(schThread *thread, int signal) {}

schSignalSet *schCreateSignal(void) {}

int schDeleteSignal(schSignalSet *signal) {}

int schBaseSignal(void) { return SIGUSR1; }

int schSignalWait(schSignalSet *sig) {}

int schSignalWaitTimeOut(schSignalSet *sig, long int nano) {}

int schSetSignalThreadMask(schSignalSet *set, int nr, const int *signals) {}

int schMutexLock(schMutex *mutex) {}

int schMutexTryLock(schMutex *mutex, long int timeout) {}

int schMutexUnLock(schMutex *mutex) {}

int schSemaphoreWait(schSemaphore *pSemaphore) {}

int schSemaphoreTryWait(schSemaphore *semaphore) {}

int schSemaphoreTimedWait(schSemaphore *pSemaphore, long int timeout) {}

int schSemaphorePost(schSemaphore *pSemaphore) {

	int schSemaphoreValue(schSemaphore * pSemaphore, int *value) {}
