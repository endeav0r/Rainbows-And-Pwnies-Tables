#include "bruteforce.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

_bruteforce * bruteforce_create (char * charset, int plaintext_length)
{
    FILE * fh;
    int filesize;
    _bruteforce * bruteforce;
    
    bruteforce = (_bruteforce *) malloc(sizeof(_bruteforce));
    
    fh = fopen(charset, "rb");
    if (fh != NULL) {
        fseek(fh, 0, SEEK_END);
        filesize = ftell(fh);
        fseek(fh, 0, SEEK_SET);

        bruteforce->charset = (char *) malloc(filesize);
        fread(bruteforce->charset, 1, filesize, fh);
        fclose(fh);

        bruteforce->charset_length = filesize;
    }
    else {
        bruteforce->charset_length = strlen(charset);
        bruteforce->charset = (char *) malloc(bruteforce->charset_length + 1);
        strcpy(bruteforce->charset, charset);
    }

    if (plaintext_length > PLAINTEXT_MAX_LEN)
        return NULL;

    bruteforce->plaintext_length = plaintext_length;
    memset(bruteforce->plaintext, 0, PLAINTEXT_MAX_LEN + 1);
    bruteforce->fast_d = libdivide_u64_gen(bruteforce->charset_length);

    if ((bruteforce->charset_length & (bruteforce->charset_length - 1)) == 0) {
        printf("charset is power of 2. speeding up bruteforce division.\n");
        bruteforce->pow2div = 1;
        while (1 << bruteforce->pow2div != bruteforce->charset_length)
            bruteforce->pow2div++;
    }
    else {
        printf("charset length %d not power of 2. using *slower* division\n",
               bruteforce->charset_length);
        bruteforce->pow2div = 0;
    }

    return bruteforce;
}


void bruteforce_destroy (_bruteforce * bruteforce)
{
    free(bruteforce->charset);
    free(bruteforce);
}


_bruteforce * bruteforce_copy (_bruteforce * src)
{
    _bruteforce * dst;

    dst = (_bruteforce *) malloc(sizeof(_bruteforce));

    dst->charset_length = src->charset_length;
    dst->plaintext_length = src->plaintext_length;
    dst->charset = (char *) malloc(dst->charset_length);
    memcpy(dst->charset, src->charset, dst->charset_length);
    memcpy(dst->plaintext, src->plaintext, PLAINTEXT_MAX_LEN + 1);
    dst->fast_d = libdivide_u64_gen(dst->charset_length);
    dst->pow2div = src->pow2div;

    return dst;
}


// this function is a huge bottleneck, mainly because of division, so we
// do whatever we can to make it faster
char * bruteforce_gen (_bruteforce * bruteforce, uint64_t seed_0, uint64_t seed_1)
{
    int i;
    int pow2div = bruteforce->pow2div;
    int pi = 0;
    uint64_t div;
    uint64_t plaintext_length = bruteforce->plaintext_length;
    uint64_t charset_length = bruteforce->charset_length;
    char * charset = bruteforce->charset;
    char * text = bruteforce->plaintext;

    for (i = 0; i < plaintext_length; i++) {
        if (pow2div)
            div = seed_0 >> pow2div;
        else
            // division is *really* slow, so we do it just once
            div = libdivide_u64_do(seed_0, &(bruteforce->fast_d));
        text[pi++] = charset[seed_0 - (charset_length * div)];
        seed_0 = div;
        if (seed_0 < charset_length)
            seed_0 = seed_1;
    }

    return text;
}
