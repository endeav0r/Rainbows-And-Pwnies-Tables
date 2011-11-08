#ifndef markov_HEADER
#define markov_HEADER

#include <stdint.h>

#include "config.h"

typedef struct _markov_s {
    int model_size;
    int num_indexes;
    unsigned char   model_indexes[256];
    unsigned char * model[256];
    unsigned char * first_chars;
    char plaintext[PLAINTEXT_MAX_LEN + 1];
    int  plaintext_length;
    int  pow2div;
    int  pow2mod;
} _markov;

_markov * markov_create  (char * filename, int plaintext_length);
void      markov_destroy (_markov * markov);

_markov * markov_copy (_markov * markov);

char *    markov_gen  (_markov * markov, uint64_t seed_0, uint64_t seed_1);

#endif
