#include "mask.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

_mask * mask_create (char * mask_text, int mask_length)
{
    int i;
    
    for (i = 0; i < strlen(mask_text); i++) {
        if (    (mask_text[i] != 'L')
             && (mask_text[i] != 'U')
             && (mask_text[i] != 'N')
             && (mask_text[i] != 'S')) {
            fprintf(stderr, "mask may only contain characters LUNS\n");
            exit(-1);
        }
    }
    _mask * mask;
    
    mask = (_mask *) malloc(sizeof(_mask));
    
    mask->mask_length = strlen(mask_text);
    mask->mask = (char *) malloc(mask->mask_length + 1);
    strcpy(mask->mask, mask_text);
    
    mask->fast_L = libdivide_u64_gen(strlen(MASK_L));
    mask->fast_N = libdivide_u64_gen(strlen(MASK_N));
    mask->fast_S = libdivide_u64_gen(strlen(MASK_S));

    return mask;
}


void mask_destroy (_mask * mask)
{
    free(mask->mask);
    free(mask);
}


_mask * mask_copy (_mask * src)
{
    _mask * new_mask;
    
    new_mask = (_mask *) malloc(sizeof(_mask));
    
    new_mask->mask_length = src->mask_length;
    new_mask->mask = (char *) malloc(strlen(src->mask) + 1);
    strcpy(new_mask->mask, src->mask);
    
    new_mask->fast_L = libdivide_u64_gen(strlen(MASK_L));
    new_mask->fast_N = libdivide_u64_gen(strlen(MASK_N));
    new_mask->fast_S = libdivide_u64_gen(strlen(MASK_S));
    
    return new_mask;
}   


char * mask_gen (_mask * mask, uint64_t seed)
{
    int i;
    int pi = 0;
    uint64_t div, sum;
    uint64_t mask_length = mask->mask_length;
    char * mask_text = mask->mask;
    char * text = mask->plaintext;

    div = 0;
    sum = seed;
    for (i = 0; i < mask_length; i++) {
        switch (mask_text[i]) {
        case 'L' :
            div = libdivide_u64_do(sum, &(mask->fast_L));
            text[pi++] = MASK_L[sum - (STRLEN_MASK_L * div)];
            break;
        case 'U' :
            div = libdivide_u64_do(sum, &(mask->fast_L));
            text[pi++] = MASK_U[sum - (STRLEN_MASK_U * div)];
            break;
        case 'N' :
            div = libdivide_u64_do(sum, &(mask->fast_N));
            text[pi++] = MASK_N[sum - (STRLEN_MASK_N * div)];
            break;
        case 'S' :
            div = libdivide_u64_do(sum, &(mask->fast_S));
            text[pi++] = MASK_S[sum - (STRLEN_MASK_S * div)];
            break;
        }
        sum = div ^ seed ^ (div << 17);
    }

    return text;
}
