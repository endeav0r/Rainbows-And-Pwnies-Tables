#ifndef bruteforce_HEADER
#define bruteforce_HEADER

#include <stdint.h>

#include "config.h"
#include "libdivide.h"

typedef struct _bruteforce_s {
    int    charset_length;
    int    plaintext_length;
    char * charset;
    char   plaintext[PLAINTEXT_MAX_LEN + 1];
    struct libdivide_u64_t fast_d;
    int    pow2div;
} _bruteforce;

_bruteforce * bruteforce_create  (char * charset, int plaintext_length);
void          bruteforce_destroy (_bruteforce * bruteforce);
_bruteforce * bruteforce_copy    (_bruteforce * src);

char * bruteforce_gen (_bruteforce * bruteforce, uint64_t seed);

#endif
