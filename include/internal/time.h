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
#ifndef _TASK_SCH_INTERNAL_TIME_H_
#define _TASK_SCH_INTERNAL_TIME_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
