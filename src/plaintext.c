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

    return plaintext;
}


void plaintext_destroy (_plaintext * plaintext)
{
    free(plaintext->charset);
    free(plaintext);
}


// this function is a huge bottleneck, mainly because of division, so we
// do whatever we can to make it faster
char * plaintext_gen (_plaintext * plaintext, uint64_t seed)
{
    int i;
    int pi = 0;
    uint64_t div, mod, diff;
    uint64_t plaintext_length = plaintext->plaintext_length;
    uint64_t charset_length = plaintext->charset_length;
    char * charset = plaintext->charset;
    char * text = plaintext->plaintext;

    if ((plaintext_length < 11) && (charset_length < 64)) {
        diff = 0x3f % charset_length;
        for (i = 0; i < plaintext_length; i++) {
            mod = seed & 0x3f;
            mod -= diff;
            if (mod >= 0x40)
                mod = seed % charset_length;
            else {
                while (mod > charset_length - 1)
                    mod -= charset_length;
            }
            text[pi++] = charset[mod];
            seed >>= 6;
            if (seed == 0)
                break;
        }
    }
    else {
        for (i = 0; i < plaintext_length; i++) {
            // division is *really* slow, so we do it just once
            div = seed / charset_length;
            text[pi++] = charset[seed - (charset_length * div)];
            seed = div;
            if (seed == 0)
                break;
        }
    } 

    text[pi] = '\0';

    return text;
}
