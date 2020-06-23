#ifndef _TASK_SCH_INTERNAL_TIME_H_
#define _TASK_SCH_INTERNAL_TIME_H_ 1

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

#endif