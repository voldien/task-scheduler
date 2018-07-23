#include"taskSch.h"

#include"taskSch.h"
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#define _MULTI_THREADED
#define _GNU_SOURCE
#define __USE_GNU

#include<pthread.h>
#include<signal.h>
#include<unistd.h>

void *schCreateThread(int affinity, void *pfunc, void *userData) {
	return 0;
}

int schDeleteThread(void *thread) {
	return 0;
}

int schWaitThread(void* thread){
	return 0;
}

void *schCurrentThread() {
	return NULL;
}

int schCreateMutex(void **mutex) {
	return 0;
}

int schDeleteMutex(void *mutex) {
	return 0;
}

int schGetNumCPUCores(void) {
	return 0;
}


int schSetThreadName(void *thread, const char *threadName) {
	return 0;
}

int schRaiseThreadSignal(void *thread, int signal) {
	return 0;
}

void *schCreateSignal(void) {
	return NULL;
}

int schBaseSignal(void){
	return 0;
}

int schSignalWait(void *sig) {
	return 0;
}

int schSignalWaitTimeOut(void *sig, long int time) {
	return 0;
}

int schSetSignalThreadMask(void *set, int nr, const int *signals) {
	return 0;
}

void schPoolLock(schTaskPool *pool) {

}

void schPoolUnLock(schTaskPool *pool) {

}