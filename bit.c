/* bit.c declares data for the bit.h macros to utilise
 *
 * written nml 2003-05-07
 *
 */

#include "bit.h"
#include <limits.h>

unsigned int bit_lbits[] = {
    0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 
    0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 
    0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff, 
    0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
#if (0xffffffffffffffff == UINT_MAX)
    , 0x1ffffffff, 0x3ffffffff, 0x7ffffffff, 0xfffffffff, 0x1fffffffff, 
    0x3fffffffff, 0x7fffffffff, 0xffffffffff, 0x1ffffffffff, 0x3ffffffffff, 
    0x7ffffffffff, 0xfffffffffff, 0x1fffffffffff, 0x3fffffffffff, 
    0x7fffffffffff, 0xffffffffffff, 0x1ffffffffffff, 0x3ffffffffffff, 
    0x7ffffffffffff, 0xfffffffffffff, 0x1fffffffffffff, 0x3fffffffffffff, 
    0x7fffffffffffff, 0xffffffffffffff, 0x1ffffffffffffff, 0x3ffffffffffffff, 
    0x7ffffffffffffff, 0xfffffffffffffff, 0x1fffffffffffffff, 
    0x3fffffffffffffff, 0x7fffffffffffffff, 0xffffffffffffffff
#elif (0xffffffffffffffff < UINT_MAX)
#error "bit.h not > 64 bit ready"
#endif
};

unsigned int bit_log2(unsigned long int num) {
    unsigned int i = 1,
                 bits = BIT_FROM_BYTE(sizeof(unsigned long int));

    while ((i < bits) && !BIT_UMASK(num, i)) {
        i++;
    }

    return bits - i;
}

#ifdef BIT_TEST
#include <stdio.h>
#include <stdlib.h>
int main() {
    printf("log2 0 %u 1 %u 2 %u UINT_MAX %u\n", bit_log2(0), bit_log2(1),
      bit_log2(2), bit_log2(UINT_MAX));
    return EXIT_SUCCESS;
}
#endif


