#include"taskSch.h"

#include<unistd.h>
#include<sys/time.h>
#include<time.h>


long int schGetTime(void) {
	struct timeval tSpec;
	gettimeofday(&tSpec, NULL);
	return (tSpec.tv_sec * 1000000 + tSpec.tv_usec) * 1000;
}

long int schTimeResolution(void) {
	struct timespec spec;
	clock_getres(CLOCK_MONOTONIC, &spec);
	return (1000000000 / spec.tv_nsec);

}