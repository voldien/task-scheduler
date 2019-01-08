#include"taskSch.h"

#include<unistd.h>
#include<sys/time.h>
#include<time.h>


long int schGetTime(void) {
	struct timeval tSpec;
	if(gettimeofday(&tSpec, NULL) == 0)
		return (tSpec.tv_sec * 1000000 + tSpec.tv_usec) * 1000;
	else{
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC_RAW, &now);
		return now.tv_sec * 1000000000 + now.tv_nsec;
	}
}

long int schTimeResolution(void) {
	struct timespec spec;
	clock_getres(CLOCK_MONOTONIC, &spec);
	return (1000000000 / spec.tv_nsec);

}