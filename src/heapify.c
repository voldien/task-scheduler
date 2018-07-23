#include"taskSch.h"
#include<string.h>
#include<limits.h>

int getParentIndex(int index, int depth) {
	return ((index - 1) / depth);
}

int getChildIndex(int parent, int nthChild, int depth) {
	return (depth * parent) + (nthChild) + 1;
}

#define HEAP_BREATH 4

void restoreDown(schTaskPool **arr, int len, int index, int k) {

	int child[k + 1];
	const long int MAX = LONG_MAX;

	while (1) {

		/*  Check for leafs.    */
		for (int i = 1; i <= k; i++)
			child[i] = ((k * index + i) < len) ?
			           (k * index + i) : -1;

		long int min_child = MAX;
		int min_child_index;

		/*  Iterate through each children and find the minimum.    */
		for (int i = 1; i <= k; i++) {
			if (child[i] != -1 &&
			    arr[child[i]]->dheapPriority < min_child) {
				min_child_index = child[i];
				min_child = arr[child[i]]->dheapPriority;
			}
		}

		/*  Leaf node.  */
		if (min_child == MAX)
			break;

		/*  */
		schTaskPool *a = arr[index];
		schTaskPool *b = arr[min_child_index];

		/*  Swap if key is the minimum. */
		if (a->dheapPriority > b->dheapPriority){
			arr[index] = b;
			arr[min_child_index] = a;
		}

		index = min_child_index;
	}
}

void schHeapify(schTaskSch *sch) {
	const int n = sch->num;
	const int d = (n - 1) / HEAP_BREATH;

	/*  Iterate through each depth layer.   */
	for (int i = d; i >= 0; i--)
		restoreDown(sch->dheap, n, i, HEAP_BREATH);
}
