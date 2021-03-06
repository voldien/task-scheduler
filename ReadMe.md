# Task Scheduler
[![Build Status](https://travis-ci.org/voldien/task-scheduler.svg?branch=master)](https://travis-ci.org/voldien/task-scheduler)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/voldien/task-scheduler.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/voldien/task-scheduler/context:cpp)

The _Task Scheduler_ is a simple task scheduler library for distributing the tasks over a set of threads of cores.

## Features ##
* Task Scheduling with a priority queue, currently a simple version based on the average delta time.
* Synchronization primitives used either with the task scheduler or with other multithreading components in the application.

## Motivation
The original purpose was to create a simple task scheduler that could be used for game engines for distributing the loads on the various individual game engine components such as frustum culling, animation and etc.

## Installation
The software can be easily installed with invoking the following command.
```bash
mkdir build && cd build
cmake ..
cmake --build .
make install
```

## Examples
The following a simple example for creating the scheduler object, running it and releasing the resources.

```c
#include<taskSch.h>

int main(int argc, const char** argv){
    schTaskSch sch;
    const size_t numPackages = 250;
    schCreateTaskPool(&sch, -1, SCH_FLAG_NO_AFM, numPackages);
	
    if(schRunTaskSch(&sch) != SCH_OK)
        return EXIT_FAILURE;
        
    schReleaseTaskSch(&sch);
    return EXIT_SUCCESS;
}

```

The following line demonstrates how to compile it and link the program:
```
gcc *.c -o task-sch-example -ltaskSch
```

## Unit Testing
The unit testing program for asserting the behavior of the program requires a set of Dependencies, see the following.
```bash
apt-get install check libsubunit-dev
```
The only dependency is the pthread for Unix machines.

## License
This project is licensed under the GPL+3 License - see the [LICENSE](LICENSE) file for details
