#ifndef plaintext_HEADER
#define plaintext_HEADER

#include <stdint.h>

#include "bruteforce.h"
#include "markov.h"

#define PLAINTEXT_TYPE_BRUTEFORCE 1
#define PLAINTEXT_TYPE_MARKOV     2

typedef struct _plaintext_s {
    int plaintext_length;
    int type;
    union {
        _markov * markov;
        _bruteforce * bruteforce;
        void * p;
    } p;
    char * (* plaintext_gen) (void * p, uint64_t seed);
} _plaintext;

// for bruteforce, s is the charset
// for markov, s is the filename of the model
_plaintext * plaintext_create  (int type, char * s, int plaintext_length);
void         plaintext_destroy (_plaintext * plaintext);
_plaintext * plaintext_copy    (_plaintext * src);

char * plaintext_gen (_plaintext * plaintext, uint64_t seed);

#endif
