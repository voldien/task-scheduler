/*
 *	Task scheduler for uniform processing in user space.
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
#ifndef _SCH_POOL_ALLOCATOR_H_
#define _SCH_POOL_ALLOCATOR_H_ 1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	Pool node element.
 */
typedef struct sch_pool_node_t {
	struct sch_pool_node_t *next; /*	Next item in the pool frame.	*/
	void *data[];				  /*	Base pointer for the element.	*/
} SchPoolNode;

/**
 * @brief Pool allocator container.
 *
 */
typedef struct sch_pool_allocator_t {
	unsigned int num;	   /*	Number of allocated elements in pool.	*/
	unsigned int itemsize; /*	Size of each element in pool.	*/
	SchPoolNode *pool;	   /*	Pool buffer.	*/
} SchPool;

/**
 * @brief Create Poll allocator.
 * [next|data]
 * @param pool
 * @param num
 * @param itemsize
 * @return non null pointer if successfully.
 */
extern SchPool *schInitPool(SchPool *pool, unsigned int num, unsigned int itemsize);

/**
 * @brief
 *
 * @param num
 * @param itemsize
 * @return SchPool*
 */
extern SchPool *schPoolCreate(unsigned int num, unsigned int itemsize);

/**
 *	Obtain the next element from pool frame.
 *
 *	If the returned  value is null,
 *	then the allocator is full.
 *
 *	@param allocator
 *	@remark: The item may not be memset to 0.
 *	@return Non null pointer if pool is not full. if null is returned, then the pool
 *	is empty.
 */
extern void *schPoolObtain(SchPool *allocator);

/**
 *	@brief Return item to pool. Item will be memset
 *	to zero.
 *
 *	@param allocator
 *	@param data
 *
 *	@return current next element in allocator.
 */
extern void *schPoolReturn(SchPool *allocator, void *data);

/**
 *	@brief Resize the current pool frame size without removing
 *	current data in the pool frame iff the num is greater
 *	than the current number of elements.
 */
extern void *schPoolResize(SchPool *allocator, unsigned int num, unsigned int itemsize);

/**
 * @brief Get the number of nodes in the pool.
 *
 * @param pool
 * @return number of nodes.
 */
extern unsigned int schPoolNumNodes(const SchPool *pool);

/**
 * @brief
 *
 * @param pool
 * @return item size in bytes
 */
extern unsigned int schPoolItemSize(const SchPool *pool);

/**
 * @brief Get the node index of a valid node.
 *
 * @param pool
 * @param data
 * @return int
 */
extern int schPoolGetIndex(const SchPool *pool, const void *data);

/**
 * @brief Free pool.
 *
 * @remark this function will call 'free' on allocator
 * and pool frame pointer. Thus, all the allocator pointer from schPoolObtain will be
 * invalid afterward.
 * @param pool valid pool object.
 */
extern void schPoolFree(SchPool *pool);

#ifdef __cplusplus
}
#endif

#endif
