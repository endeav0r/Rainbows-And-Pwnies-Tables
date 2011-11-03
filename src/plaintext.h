#ifndef plaintext_HEADER
#define plaintext_HEADER

#include <stdint.h>

#include "libdivide.h"

#define PLAINTEXT_MAX_LEN 32

typedef struct _plaintext_s {
    int charset_length;
    int plaintext_length;
    char * charset;
    char plaintext[PLAINTEXT_MAX_LEN + 1];
    struct libdivide_u64_t fast_d;
} _plaintext;


_plaintext * plaintext_create  (char * charset, int plaintext_length);
void         plaintext_destroy (_plaintext * plaintext);
_plaintext * plaintext_copy    (_plaintext * src);

char * plaintext_gen (_plaintext * plaintext, uint64_t seed);

#endif
