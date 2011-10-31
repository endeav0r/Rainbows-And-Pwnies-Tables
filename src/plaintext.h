#ifndef plaintext_HEADER
#define plaintext_HEADER

#include <stdint.h>

#define PLAINTEXT_MAX_LEN 32

typedef struct _plaintext_s {
    int charset_length;
    int plaintext_length;
    char * charset;
    char plaintext[PLAINTEXT_MAX_LEN + 1];
} _plaintext;


_plaintext * plaintext_create  (char * charset, int plaintext_length);
void         plaintext_destroy (_plaintext * plaintext);

char * plaintext_gen (_plaintext * plaintext, uint64_t seed);

#endif
