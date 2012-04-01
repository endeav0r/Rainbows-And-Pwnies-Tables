#ifndef mask_HEADER
#define mask_HEADER

#include <stdint.h>

#include "config.h"
#include "libdivide.h"

#define MASK_L "abcdefghijklmnopqrstuvwxyz"
#define MASK_U "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define MASK_N "0123456789"
//              0        1         2
//              12345678901234567890123456 7
#define MASK_S "!@#$%^&*()_+{}<>?,./;'[]-=\""

#define STRLEN_MASK_L 26
#define STRLEN_MASK_U 26
#define STRLEN_MASK_N 10
#define STRLEN_MASK_S 27

typedef struct _mask_s {
    int    mask_length;
    char * mask;
    char   plaintext[PLAINTEXT_MAX_LEN + 1];
    struct libdivide_u64_t fast_L;
    struct libdivide_u64_t fast_N;
    struct libdivide_u64_t fast_S;
} _mask;

_mask * mask_create  (char * mask_text, int plaintext_length);
void    mask_destroy (_mask * mask);
_mask * mask_copy    (_mask * src);

char * mask_gen (_mask * mask, uint64_t seed);

#endif
