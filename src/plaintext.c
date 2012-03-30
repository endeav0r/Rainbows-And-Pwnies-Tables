#include "plaintext.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

_plaintext * plaintext_create (int type, char * s, int plaintext_length)
{
    _plaintext * plaintext;

    if (plaintext_length > PLAINTEXT_MAX_LEN)
        return NULL;

    plaintext = (_plaintext *) malloc(sizeof(_plaintext));

    plaintext->plaintext_length = plaintext_length;
    plaintext->type = type;

    switch (type) {
    case PLAINTEXT_TYPE_BRUTEFORCE :
        plaintext->p.bruteforce = bruteforce_create(s, plaintext_length);
        plaintext->plaintext_gen = bruteforce_gen;
        break;
    case PLAINTEXT_TYPE_MARKOV :
        plaintext->p.markov     = markov_create(s, plaintext_length);
        plaintext->plaintext_gen = markov_gen;
        break;
    default :
        fprintf(stderr, "invalid plaintext type\n");
        return NULL;
    }

    return plaintext;
}


void plaintext_destroy (_plaintext * plaintext)
{
    switch (plaintext->type) {
    case PLAINTEXT_TYPE_BRUTEFORCE :
        bruteforce_destroy(plaintext->p.bruteforce);
        break;
    case PLAINTEXT_TYPE_MARKOV :
        markov_destroy(plaintext->p.markov);
        break;
    }
    free(plaintext);
}


_plaintext * plaintext_copy (_plaintext * src)
{
    _plaintext * dst;

    dst = (_plaintext *) malloc(sizeof(_plaintext));

    dst->plaintext_length = src->plaintext_length;
    dst->type             = src->type;
    dst->plaintext_gen    = src->plaintext_gen;

    switch (src->type) {
    case PLAINTEXT_TYPE_BRUTEFORCE :
        dst->p.bruteforce = bruteforce_copy(src->p.bruteforce);
        break;
    case PLAINTEXT_TYPE_MARKOV :
        dst->p.markov = markov_copy(src->p.markov);
        break;
    }

    return dst;
}


// this function is a huge bottleneck, mainly because of division, so we
// do whatever we can to make it faster
char * plaintext_gen (_plaintext * plaintext, uint64_t seed)
{
    return plaintext->plaintext_gen(plaintext->p.p, seed);
}
