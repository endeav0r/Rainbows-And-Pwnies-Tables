#include "plaintext.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

_plaintext * plaintext_create (char * charset, int plaintext_length)
{
    _plaintext * plaintext;

    if (plaintext_length > PLAINTEXT_MAX_LEN)
        return NULL;

    plaintext = (_plaintext *) malloc(sizeof(_plaintext));

    plaintext->charset_length = strlen(charset);
    plaintext->plaintext_length = plaintext_length;
    plaintext->charset = (char *) malloc(plaintext->charset_length + 1);
    strcpy(plaintext->charset, charset);
    memset(plaintext->plaintext, 0, PLAINTEXT_MAX_LEN + 1);
    plaintext->fast_d = libdivide_u64_gen(plaintext->charset_length);

    return plaintext;
}


void plaintext_destroy (_plaintext * plaintext)
{
    free(plaintext->charset);
    free(plaintext);
}


_plaintext * plaintext_copy (_plaintext * src)
{
    _plaintext * dst;

    dst = (_plaintext *) malloc(sizeof(_plaintext));

    dst->charset_length = src->charset_length;
    dst->plaintext_length = src->plaintext_length;
    dst->charset = (char *) malloc(dst->charset_length);
    memcpy(dst->charset, src->charset, dst->charset_length);
    memcpy(dst->plaintext, src->plaintext, PLAINTEXT_MAX_LEN + 1);
    dst->fast_d = libdivide_u64_gen(dst->charset_length);

    return dst;
}


// this function is a huge bottleneck, mainly because of division, so we
// do whatever we can to make it faster
char * plaintext_gen (_plaintext * plaintext, uint64_t seed)
{
    int i;
    int pi = 0;
    uint64_t div;
    uint64_t plaintext_length = plaintext->plaintext_length;
    uint64_t charset_length = plaintext->charset_length;
    char * charset = plaintext->charset;
    char * text = plaintext->plaintext;

    for (i = 0; i < plaintext_length; i++) {
        // division is *really* slow, so we do it just once
        div = libdivide_u64_do(seed, &(plaintext->fast_d));
        text[pi++] = charset[seed - (charset_length * div)];
        seed = div;
        if (seed == 0)
            break;
    }

    text[pi] = '\0';

    return text;
}
