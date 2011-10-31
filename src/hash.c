#include "hash.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

_hash * hash_create (int sum_size, HASH_FUNC)
{
    _hash * hash;

    hash = (_hash *) malloc(sizeof(_hash));
    hash->sum_size = sum_size;
    hash->hash_func = hash_func;

    return hash;
}


void hash_destroy (_hash * hash)
{
    free(hash);
}


void hash_hash (_hash * hash, unsigned char * data, int data_len)
{
    hash->hash_func(hash->sum, data, data_len);
}


int hash_from_string (_hash * hash, char * string)
{
    int i;
    int high;
    int low;

    if (strlen(string) & 1)
        return -1;

    for (i = 0; i < strlen(string); i += 2) {
        if ((string[i] >= 'a') && (string[i] <= 'f'))
            high = string[i] - 'a' + 10;
        else if ((string[i] >= '0') && (string[i] <= '9'))
            high = string[i] - '0';
        else
            return -1;

        if ((string[i+1] >= 'a') && (string[i+1] <= 'f'))
            low = string[i+1] - 'a' + 10;
        else if ((string[i+1] >= '0') && (string[i+1] <= '9'))
            low = string[i+1] - '0';
        else
            return -1;

        hash->sum[i/2]  = (char) (high << 4);
        hash->sum[i/2] |= (char) low;
    }

    return 0;
}
        


uint64_t hash_index(_hash * hash)
{
    uint64_t index;
    int i;

    index = 0;

    for (i = 0; i < hash->sum_size; i++) {
        switch (i & 7) {
        case 0 :
            index ^= (uint64_t) hash->sum[i];
            break;
        case 1 :
            index ^= ((uint64_t ) hash->sum[i]) << 8;
            break;
        case 2 :
            index ^= ((uint64_t ) hash->sum[i]) << 16;
            break;
        case 3 :
            index ^= ((uint64_t ) hash->sum[i]) << 24;
            break;
        case 4 :
            index ^= ((uint64_t ) hash->sum[i]) << 32;
            break;
        case 5 :
            index ^= ((uint64_t ) hash->sum[i]) << 40;
            break;
        case 6 :
            index ^= ((uint64_t ) hash->sum[i]) << 48;
            break;
        case 7 :
            index ^= ((uint64_t ) hash->sum[i]) << 56;
            break;
        }
    }

    return index;
}
