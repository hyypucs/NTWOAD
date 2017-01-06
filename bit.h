/* bit.h declares a lot of macros to do bit manipulation in a
 * portable way
 *
 * make sure the variables you're operating on are *unsigned*, else
 * the results are undefined 
 *
 * written nml 2003-04-29
 *
 */

#ifndef BIT_H
#define BIT_H

#include <assert.h>
#include <string.h>

extern unsigned int bit_lbits[];

/* basic functions */

/* bit_set sets bit number bit in var to val (0 is lsb), evaluating to the
 * result (var is not changed in place) */
#define BIT_SET(var, bit, val) \
  (assert(((bit) >= 0) && ((bit) < (8 * sizeof(unsigned int))), \
    (((var) & ~(1 << (bit))) | (!!(val) << (bit))))

/* bit_clear clears bit number bit in var (0 is lsb), evaluating to
 * the result (var is not changed in place) */
#define BIT_CLEAR(var, bit) \
  (assert(((bit) >= 0) && ((bit) < (8 * sizeof(unsigned int)))), \
    ((var) & ~(1 << (bit))))

/* bit_toggle toggles bit number bit in var (0 is lsb), evaluating to
 * the result (var is not changed in place) */
#define BIT_TOGGLE(var, bit) \
  (assert(((bit) >= 0) && ((bit) < (8 * sizeof(unsigned int)))), \
    ((var) ^ (1 << (bit))))

/* bit_get returns bit number bit in var (0 is lsb) */
#define BIT_GET(var, bit) \
  (assert(((bit) >= 0) && ((bit) < (8 * sizeof(unsigned int)))), \
    ((var) & (1 << (bit))))

/* strength-reduced arithmetic */

/* bit_mul2 performs integer multiplication between num and a power of two 
 * (specified by pow) */
#define BIT_MUL2(num, pow) ((num) << (pow))

/* bit_div2 performs integer division between num and a power of two
 * (specified by pow) */
#define BIT_DIV2(num, pow) ((num) >> (pow))

/* bit_mod2 performs integer modulus between num and a power of two
 * (specified by pow) */
#define BIT_MOD2(num, pow) (BIT_LMASK((num), (pow)))

/* bit_pow2 performs integer exponentiation of a power of
 * two (specified by pow) (pow must be between 0 and the wordsize of
 * your computer in bits - 1, else 0 is returned) */
#define BIT_POW2(pow) \
  ((((pow) >= 0) && ((pow) < (8 * sizeof(unsigned int)))) \
    ? (bit_lbits[(pow)] + 1) : 0)

/* bit_log2 returns floor(log_2(num)) except if num is 0, in which case 0 is
 * returned. */
unsigned int bit_log2(unsigned long int num);

/* masking */

/* return the upper bits bits of num */
#define BIT_UMASK(num, bits) \
  (assert(((bits) >= 0) && ((bits) < (8 * sizeof(unsigned int)))), \
    (num) & ~bit_lbits[(8 * (sizeof(unsigned int))) - (bits)])

/* return the lower bits bits of num */
#define BIT_LMASK(num, bits) \
  (assert(((bits) >= 0) && ((bits) < (8 * sizeof(unsigned int)))), \
    (num) & bit_lbits[(bits)])

/* pointer setting (this version is non-portable) */
#define BIT_ARRAY_NULL(arr, size) (memset((arr), 0, (size) * sizeof((arr)[0])))

/* bits to bytes and vice versa */

/* return floor(number of bytes in given bits) */
#define BIT_TO_BYTE(bits) (BIT_DIV2((bits), 3))

/* return number of bits in given bytes */
#define BIT_FROM_BYTE(bytes) (BIT_MUL2((bytes), 3))

#endif

