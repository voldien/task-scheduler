# Task Scheduler

[![License: LGPL v3](https://img.shields.io/badge/License-LGPLv3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![GitHub release](https://img.shields.io/github/release/voldien/task-scheduler.svg)](https://github.com/voldien/task-scheduler/releases)

The _Task Scheduler_ is a simple task scheduler library for distributing the tasks over a set of threads of cores.

## Features

* **Task Scheduling** - with a priority queue, currently a simple version based on the average delta time.
* **Synchronization primitives** - used either with the task scheduler or with other multithreading components in the application.

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

## Development Build

For building for development, the following commands can be used.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Integration with CMake

The idea is to be able to integrate this library with another project easily. With CMake, it basically requires 2 lines. One for adding the project and the second for adding it as a dependent linked library target.

```cmake
ADD_SUBDIRECTORY(task-scheduler EXCLUDE_FROM_ALL)
```

```cmake
TARGET_LINK_LIBRARIES(myTarget PUBLIC taskSch)
```

## Examples

The following a simple example for creating the scheduler object, running it and releasing the resources.

```c
#include<taskSch.h>

int main(int argc, const char** argv){
    schTaskSch* sch;
    const size_t numPackages = 250;

    // Allocate memory for the internal structure.
    schAllocateTaskPool(&sch);

    // Create the task pool and its internal structure.
    schCreateTaskPool(sch, -1, SCH_FLAG_NO_AFM, numPackages);

    // Start running the scheduler, if failed exit
    if(schRunTaskSch(sch) != SCH_OK)
        return EXIT_FAILURE;
        
    // Release scheduler internal structures
    schReleaseTaskSch(sch);
    return EXIT_SUCCESS;
}

```

The following line demonstrates how to compile it and link the program, using gcc:

```bash
gcc *.c -o task-sch-example -ltaskSch
```

## Unit Testing

The unit testing program for asserting the behavior of the program requires a set of Dependencies, see the following.

```bash
apt-get install check libsubunit-dev
```

The only dependency is the pthread for Unix machines.

## License

This project is licensed under the LGPL+3 License - see the [LICENSE](LICENSE) file for more details.
