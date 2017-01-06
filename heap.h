/* heap.h declares some implicit minheap functions and a partial sorting 
 * function that uses the classic heapsort to work its magic :o)
 *
 * note that this heap stores the most extreme (smallest) element at the back 
 * of the array so that heap_sort can partially sort results into the *front* 
 * of the array.  This was done in order to ensure that partially sorted 
 * results occur at the front of the heap.  Unfortunately i've since 
 * discovered a better way of doing this (instead of heapifying the entire 
 * array, just heapify the number of elements you want.  Compare the rest of 
 * the elements with the most extreme element in the heap, swapping them in 
 * if they are less extreme.  Then sort the heap.  Note that this requires 
 * creative reversal of the comparison function).  So this is a little 
 * complicated and redundant, so i'll have to FIXME at some stage.
 *
 * written nml 2003-06-03
 *
 */

#ifndef HEAP_H
#define HEAP_H

#ifdef _cplusplus
extern "C" {
#endif

#include <stdlib.h>

/* heap sort sorts the given array (array can be in any order).  base is a 
 * pointer to the start of the
 * array, nmemb is the number of elements in the array, sort is the number of
 * elements in the array that you would like sorted [0, nmemb], 
 * size is the size of each individual element (shouldn't be 0), and compar is 
 * a comparison function for elements.  After execution, the top sort elements 
 * of the array will be in sorted order, and will be smaller than the rest of 
 * the elements. */
void heap_sort(void *base, size_t nmemb, size_t sort, size_t size, 
  int (*compar)(const void *, const void *));

/* heapify creates an implicit heap from the given array by moving elements
 * around within the array */
void heap_heapify(void *base, size_t nmemb, size_t size, 
  int (*compar)(const void *, const void *));

/* replace replaces the root (smallest) element of the heap, and then returns
 * the new position of element within the heap.  Old root is copied into 
 * element */
void *heap_replace(void *base, size_t nmemb, size_t size,
  int (*compar)(const void *, const void *), void *element);

/* returns a pointer to the smallest element in the heap */
void *heap_peek(void *base, size_t nmemb, size_t size);

#if 0
/* these functions aren't implemented yet */

/* remove the smallest element from the heap */
void *heap_pop(void *base, size_t *nmemb, size_t size);

/* insert inserts element into the heap, growing it by size in the process
 * (nmemb will be updated to reflect the new number of elements).  heap_insert
 * realloc's the array to hold all of the elements. */
void *heap_insert(void *base, size_t *nmemb, size_t size,
  int (*compar)(const void *, const void *), void *element);

/* remove removes element from the heap, shrinking it by size in the process
 * (nmemb will be updated to reflect the new number of elements) */
void *heap_remove(void *base, size_t *nmemb, size_t size,
  int (*compar)(const void *, const void *), void *element);

/* find searches the heap for the given element */
void *heap_find(void *base, size_t nmemb, size_t size, 
  int (*compar)(const void *, const void *), void *element);
#endif

#ifdef _cplusplus
}
#endif

#endif

