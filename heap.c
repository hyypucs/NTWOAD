/* heap.c implements an efficient partial sort using a heap.
 *
 * heaps are a classic computer science structure.  If you don't know what one
 * is i refer you to The Art of Computer Programming, Volume 3, Donald E Knuth.
 * For a less in-depth but more readable resource you could check out the 
 * Wikipedia entry on heaps (http://www.wikipedia.org/wiki/Heap), binary heaps
 * (http://www.wikipedia.org/wiki/Binary_heap), and 
 * heapsort (http://www.wikipedia.org/wiki/Heapsort).
 *  
 * I'll summarise the heap structure so that i can explain some potential
 * optimisations to this code.  A heap is a tree having the property that each
 * node is more extreme (greater than or less than, depending whether it is a
 * minheap or a maxheap) or equal to its children nodes.  This code implements a
 * minheap, so from now on i'll just refer to parent nodes being <= their 
 * children.  The minheap property ensures that the (possibly equal) smallest 
 * node is at the root of the tree, so its an O(1) operation to get the 
 * smallest node from a heap.  Restoring the heap property to a heap minus its
 * smallest element takes O(logN) time.  Heaps can be constructed in O(N) time 
 * given any old bunch of elements.  
 *
 * Heapsort is essentially these 4 steps:
 *    1) make a heap from the elements
 *    2) for each element in heap
 *    3)   remove smallest element
 *    4)   restore heap property to remaining heap
 *
 * we can see from the above complexity measures that this is an 
 * O(N) (to build heap) + O(NlogN) (to reheapify N times) = O(NlogN) operation.
 * Heapsort is typically not as fast as quicksort, but its worst case is
 * O(NlogN), which is far better than quicksort's O(N^2) worst case.
 * Heapsort can also be made to work in-place, with only one byte of extra
 * storage required (which this implementation does).  It also has the
 * interesting property that you can modify step 2 to only iterate a set number
 * of times, which will give you what is known as a partial sort (where you
 * extract the M smallest values in sorted order from your heap).
 *
 * Binary heaps (where each node has two children) can be represented by an
 * array, (numbered from 1 ... N for convenience) where each node n has children
 * n * 2 and n * 2 + 1.  E.g. a heap (ASCII art from wikipedia)
 * 
 *         1
 *        / \
 *       /   \
 *      4     3
 *     / \   / \
 *    5   6 7   8
 *
 * could be represented as array [1, 4, 3, 5, 6, 7, 8].  I have complicated
 * things in this code by reversing the order of the array (this was done to
 * ensure that the partially sorted results occur at the front of the array,
 * instead of the back - more on this later).  So my representation would be
 * [8, 7, 6, 5, 3, 4, 1], but i will use indeces starting with the root at 1
 * to be less confusing.
 *
 * Given an array of elements, we can construct a heap from it.  Start at
 * element floor(N / 2) (since this is the first element that can have
 * children).  Find out which child is smaller (the first node might not have 
 * both children) and compare the smaller child with the parent.  If the child
 * is smaller than the parent, swap them.  After a swap, you need to recursively
 * repeat this process with the subheap formed by the newly swapped child and 
 * its children to ensure the heap property.  After finishing with node floor(N
 * / 2), repeat with the previous node (floor(N / 2) - 1) until the first node
 * has been heapified as well.  You now have a heap.
 *
 * The second stage of heapsort involves repeatedly removing the smallest
 * element and reheapifying.  Reheapification is exactly the same recursive
 * process as we applied to the first half of the array to make the heap, and is
 * known as siftup or siftdown.  With our array representation, we swap the 
 * smallest element with the last element and shrink the size of the heap by 
 * one.  As the heap shrinks, we will accumulate a set of the smallest 
 * elements, in order, behind the heap (which is exactly what we want).  So, 
 * with an example: 
 *
 * start:                       [1, 4, 3, 5, 6, 7, 8]
 * swap smallest and last:      [8, 4, 3, 5, 6, 7], 1 
 * siftdown:                    [3, 4, 8, 5, 6, 7], 1
 *                              [3, 4, 7, 5, 6, 8], 1
 * swap smallest and last:      [8, 4, 7, 5, 6], 3, 1 
 * siftdown:                    [4, 8, 7, 5, 6], 3, 1
 *                              [4, 5, 7, 8, 6], 3, 1
 * swap smallest and last:      [6, 5, 7, 8], 4, 3, 1
 * siftdown:                    [5, 6, 7, 8], 4, 3, 1
 * swap smallest and last:      [8, 6, 7], 5, 4, 3, 1
 * siftdown:                    [6, 8, 7], 5, 4, 3, 1
 * swap smallest and last:      [7, 8], 6, 5, 4, 3, 1
 * siftdown:                    [7, 8], 6, 5, 4, 3, 1
 * swap smallest and last:      8, 7, 6, 5, 4, 3, 1         (finished)
 *
 * note how the heap shrunk and the sorted array at the back grows.  This
 * implementation has the heap at the back and the sorted array at the front,
 * but the algorithm is the same.
 *
 * This code implements a basic heapsort.  It sorts about twice as slowly as the
 * standard library qsort on my machine at the moment.  I believe that this is
 * acceptible performance for the moment.  There are a number of optimisations
 * that should make it faster:
 *
 *   - improved siftup algorithm
 *   - altering code to be a ternary heap
 *
 * During the siftup algorithm, instead of finding the smallest child and
 * comparing it to the parent, you can find the smallest child and swap it with
 * the parent.  This propagates the parent to the bottom of the heap, from where
 * you have to compare it with its (new) parents and swap it back up to find its
 * place in the heap.  While this involves more copying of elements, it
 * apparently saves comparisons, and should improve the performance of this heap
 * (particularly with a generic comparison function pointer and in-line
 * swapping).  The downside to this is that i can't find a way to do it with
 * fast maths.
 *
 * A ternary heap (where each parent has 3 children) is apparently faster than a
 * binary heap (and fast math can still be done: x * 3 == (x << 1) + x).
 *
 * Another algorithmic improvement might be to implement a weak-heap as
 * described by Dr Stefan Edelkamp in his master's thesis: Weak-Heapsort: A Fast * Sorting Algorithm, Institute of Computer Science, University of Dortmund.
 * Unfortunately its in german, and there doesn't seem to be a lot of english
 * literature on it.  It utilises a weak heap, which is a tree where each parent
 * is more extreme or equal to the elements in its right subtree, and the root
 * has no left subtree (this constraint is weaker than the heap constraint, but
 * is still sufficient to sort the elements, apparently).  Unfortunately it
 * requires an additional array or r bits, which indicate which elements an
 * element considers its right subtree.
 *
 * written nml 2003-06-03
 *
 */

#include "heap.h"
#include "bit.h"

#ifndef NDEBUG
/* internal function to determine whether an array has the heap property */
static int heap_isheap(void *base, size_t nmemb, size_t size, 
  int (*cmp)(const void *, const void *)) {
    unsigned char *cbase = (unsigned char *)base;
    unsigned int pos = BIT_DIV2(nmemb, 1) - 1,
                 lchild,
                 rchild;

    /* heap property: every node is less than (or equal to) its children */

    if (nmemb < 2) {
        return 1;
    }

    for (pos = nmemb - 1, lchild = pos - 1, rchild = lchild - 1; 
      pos > BIT_DIV2(nmemb, 1); pos--, lchild -= 2, rchild -= 2) {
        if ((cmp(&cbase[pos * size], &cbase[lchild * size]) > 0) 
          || (cmp(&cbase[pos * size], &cbase[rchild * size]) > 0)) { 
            return 0;
        }
    }

    /* last comparison */
    if (lchild < nmemb) {
        if (cmp(&cbase[pos * size], &cbase[lchild * size]) > 0) {
            return 0;
        }
    }
    
    return 1;
}

/* internal function to determine whether an array is sorted */
static int heap_issorted(void *base, size_t nmemb, size_t size, 
  int (*cmp)(const void *, const void *)) {
    unsigned char *pos,
                  *next,
                  *end = ((unsigned char *) base) + nmemb * size;

    /* sorted property: every node is smaller than or equal to the next */

    if (nmemb < 2) {
        return 1;
    }

    for (pos = (unsigned char *)base, next = pos + size; next < end; pos = next, next += size) {
        if (cmp(next, pos) < 0) {
            return 0;
        }
    }

    return 1;
}
#endif

/* old macro to swap two data elements in place, one byte at a time.  I tried
 * replacing this with a duff device for optimisation, but it had no noticeable
 * effect. */
#define SWAP(one, two, size)                                                  \
    do {                                                                      \
        unsigned char SWAP_tmp,                                               \
                      *SWAP_tone = (unsigned char*) one,                                       \
                      *SWAP_ttwo = (unsigned char*) two;                                       \
        size_t SWAP_size = size;                                              \
        do {                                                                  \
            SWAP_tmp = *SWAP_tone;                                            \
            *SWAP_tone = *SWAP_ttwo;                                          \
            *SWAP_ttwo = SWAP_tmp;                                            \
            ++SWAP_tone;                                                      \
            ++SWAP_ttwo;                                                      \
        } while (--SWAP_size);                                                \
    } while (0)

/* internal function to reheapify a heap with the top element out of place.
 * base is start of the array, size is the size of each element, 
 * cmp compares elements and initdiff gives the number of bytes until the left 
 * child of the base element.  I tried inlining this function but it was only 4%
 * faster, much nastier and wasn't fully debugged, so it was kept as a 
 * function.  */
static void *siftup(void *vpos, void *vstart, size_t size, 
  int (*cmp)(const void *, const void *), unsigned int initdiff) { 
    unsigned char *start = (unsigned char*) vstart,
                  *pos = (unsigned char *) vpos,
                  *lchild = pos - initdiff,
                  *rchild = lchild - size;
    unsigned int diff = initdiff;

    /* perform siftups where both children are in the heap */
    while (rchild >= start) {
        if (cmp(lchild, rchild) < 0) {
            /* lchild is smaller */
            if (cmp(lchild, pos) < 0) {
                /* lchild is still smaller, swap for parent */
                SWAP(lchild, pos, size);

                /* sift-down to reheapify left heap */

                /* the space to the next child doubles as we go down lchild */
                diff = BIT_MUL2(diff, 1);

                /* iterate down */
                pos = lchild;
                lchild -= diff;
                rchild = lchild - size;
            } else {
                return pos;
            }
        } else {
            /* rchild is smaller */
            if (cmp(rchild, pos) < 0) {
                /* rchild is still smaller, swap for parent */
                SWAP(rchild, pos, size);

                /* sift-down to reheapify right heap */

                /* the space to the next child doubles as we go down, but
                 * because its the rchild we need to skip one extra as well */
                diff = BIT_MUL2(diff, 1) + size;

                /* iterate down */
                pos = rchild;
                lchild = rchild - diff;
                rchild = lchild - size;
            } else {
                return pos;
            }
        }
    }

    /* perform last siftup, where left child may be in the heap */
    if (lchild >= start) {
        /* lchild is smaller */
        if (cmp(lchild, pos) < 0) {
            /* lchild is still smaller, swap for parent */
            SWAP(lchild, pos, size);
            /* don't have to siftup because its a base subheap */
            return lchild;
        }
    }

    return pos;
}

void heap_sort(void *base, size_t nmemb, size_t sort, size_t size, 
  int (*cmp)(const void *, const void *)) {
    unsigned char *head = (unsigned char*) base,
                  *end = head + nmemb * size,
                  *last = end - size;
    unsigned int tosort = sort;

    /* we don't have to sort 'degenerate' arrays */
    if (!sort || !size || (nmemb < 2)) {
        return;
    }

    if (sort > nmemb) {
        sort = nmemb;
    }

    /* create a heap */
    heap_heapify(base, nmemb, size, cmp);

    /* copy root element out of heap and siftup */
    while (tosort) {
        SWAP(head, last, size);
        head += size;
        tosort--;
        siftup(last, head, size, cmp, size);
    }

    /* if debugging is on, check the sort order */
    assert(heap_issorted(base, sort, size, cmp));

    /* the number of elements they requested should now be sorted */
    return;
}

void heap_heapify(void *base, size_t nmemb, size_t size, 
  int (*cmp)(const void *, const void *)) {
    unsigned int ipos;
    unsigned char *cbase = (unsigned char*) base,
                  *end = cbase + nmemb * size,
                  *pos,
                  *lchild,
                  *rchild;

    if (nmemb < 2) {
        /* arrays of size 1 or 0 are degenerate heaps already */
        return;
    }

    /* initialise positions */
    ipos = BIT_DIV2(nmemb, 1) - 1;
    pos = cbase + (nmemb - (ipos + 1)) * size;
    lchild = cbase + (nmemb - (1 + (1 + BIT_MUL2(ipos, 1)))) * size;
    rchild = lchild - size;

    /* do first heapification, where right child might be in the heap */
    if (rchild >= cbase) {
        if (cmp(lchild, rchild) < 0) {
            /* lchild is smaller */
            if (cmp(lchild, pos) < 0) {
                /* lchild is still smaller, swap for parent
                 * (no siftup, because its a base subheap) */
                SWAP(lchild, pos, size);
            }
        } else {
            /* rchild is larger */
            if (cmp(rchild, pos) < 0) {
                /* rchild is still smaller, swap for parent 
                 * (no siftup, because its a base subheap) */
                SWAP(rchild, pos, size);
            }
        }
    } else {
        if (cmp(lchild, pos) < 0) {
            /* lchild smaller, swap for parent 
             * (no siftup, because its a base subheap) */
            SWAP(lchild, pos, size);
        }
    }

    /* iterate down */
    pos += size;
    lchild += size + size;
    rchild += size + size;

    /* perform all subsequent heapifications */
    while (pos < end) {
        if (cmp(lchild, rchild) < 0) {
            /* lchild is smaller */
            if (cmp(lchild, pos) < 0) {
                /* lchild is still smaller, swap for parent */
                SWAP(lchild, pos, size);
                siftup(lchild, base, size, cmp, BIT_MUL2(pos - lchild, 1));
            }
        } else {
            /* rchild is smaller */
            if (cmp(rchild, pos) < 0) {
                /* rchild is still smaller, swap for parent */
                SWAP(rchild, pos, size);
                siftup(rchild, base, size, cmp, 
                  BIT_MUL2(pos - lchild, 1) + size);
            }
        }
 
        pos += size;
        lchild += size + size;
        rchild += size + size;
    }

    /* if debugging is on, check the heap order */
    assert(heap_isheap(base, nmemb, size, cmp));

    return;
}

void *heap_replace(void *base, size_t nmemb, size_t size,
  int (*cmp)(const void *, const void *), void *element) {
    unsigned char *cbase = (unsigned char*) base,
                  *root = cbase + (nmemb - 1) * size;

    /* swap root out */
    SWAP(root, element, size);

    /* reheapify */
    return siftup(root, base, size, cmp, size);
}

void *heap_peek(void *base, size_t nmemb, size_t size) {
    /* smallest is last in (root of) heap */
    unsigned char *cbase = (unsigned char*) base;
    return cbase + (nmemb - 1) * size;
}

/* module test code */
#ifdef HEAP_TEST
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

static void printarr(unsigned int *arr, unsigned int elements) {
    unsigned int i;

    for (i = 0; i < elements; i++) {
        fprintf(stderr, "%u %u\n", i, arr[i]);
    }
    fprintf(stderr, "\n");
    return;
}

static void fillarr(unsigned int *arr, unsigned int elements, 
  unsigned int seed) {
    unsigned int i;

    for (srand(seed), i = 0; i < elements; i++) {
        arr[i] = rand() % (elements << 1);
    }

    return;
}

static int cmp_int(const void *vone, const void *vtwo) {
    const unsigned int *one = vone,
                       *two = vtwo;

    if (*two < *one) {
        return 1;
    } else if (*one < *two) {
        return -1;
    } else {
        return 0;
    }
}

int main(int argc, char **argv) {
    unsigned int elements;
    unsigned int seed;
    unsigned int *arr;

    seed = time(NULL);

    if ((argc == 2) || (argc == 3)) {
        elements = strtol(argv[1], NULL, 0);
    } else {
        fprintf(stderr, "usage: %s elements [seed]\n", *argv);
        return EXIT_FAILURE;
    }

    if (argc == 3) {
        seed = strtol(argv[2], NULL, 0);
    }

    if (!(arr = malloc(sizeof(*arr) * elements))) {
        fprintf(stderr, "can't get memory\n");
    }

    fillarr(arr, elements, seed);

    /* sort */
    heap_sort(arr, elements, elements, sizeof(*arr), cmp_int);

    if (!heap_issorted(arr, elements, sizeof(*arr), cmp_int)) {
        printarr(arr, elements);
        printf("arr is not sorted (seed %u)!\n", seed);
    } 

    free(arr);

    return EXIT_SUCCESS;
}
#endif

