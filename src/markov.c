#include "markov.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_markov * markov_create (char * filename, int plaintext_length)
{
    FILE * fh;
    size_t filesize;
    int model_size;
    int i;
    _markov * markov;
    unsigned char * filebuf;

    fh = fopen(filename, "r");
    if (fh == NULL) {
        fprintf(stderr, "failed to open file %s\n", filename);
        return NULL;
    }

    fseek(fh, 0, SEEK_END);
    filesize = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    filebuf = (unsigned char *) malloc(filesize);
    fread(filebuf, 1, filesize, fh);

    fclose(fh);

    // length of first line is the size of the model
    model_size = -1;
    for (i = 0; i < filesize; i++) {
        // windows line endings beware
        if (filebuf[i] == '\n') {
            model_size = i;
            break;
        }
    }

    if (model_size < 0) {
        fprintf(stderr, "invalid model file\n");
        return NULL;
    }

    // make sure user didn't fuck up power of 2
    if ((model_size & (model_size - 1)) != 0) {
        fprintf(stderr, "model size must be power of 2\n");
        return NULL;
    }

    markov = (_markov *) malloc(sizeof(_markov));
    markov->model_size = model_size;
    markov->first_chars = malloc(model_size);
    markov->plaintext_length = plaintext_length;
    memcpy(markov->first_chars, filebuf, model_size);
    markov->num_indexes = 0;

    for (i = 0; i < 256; i++)
        markov->model[i] = NULL;

    // start after first newline
    i = model_size + 1;
    while (i < filesize) {
        // i is index of key character
        markov->model_indexes[markov->num_indexes++] = filebuf[i];
        markov->model[filebuf[i]] = (unsigned char *) malloc(model_size);
        memcpy(markov->model[filebuf[i]], &(filebuf[i+1]), model_size);
        i += model_size + 2;
    }

    // calculate values for fast "division"
    markov->pow2div = 1;
    while ((1 << markov->pow2div) != model_size)
        markov->pow2div++;

    markov->pow2mod = model_size - 1;

    free(filebuf);
    return markov;
}


void markov_destroy (_markov * markov)
{
    int i;

    for (i = 0; i < 256; i++)
        free(markov->model[i]);

    free(markov->first_chars);

    free(markov);
}


_markov * markov_copy (_markov * src)
{
    int i;
    _markov * dst;

    dst = (_markov *) malloc(sizeof(_markov));

    dst->model_size = src->model_size;
    dst->num_indexes = src->num_indexes;
    memcpy(dst->model_indexes, src->model_indexes, 256);

    dst->first_chars = (unsigned char *) malloc(dst->model_size);
    memcpy(dst->first_chars, src->first_chars, dst->model_size);

    for (i = 0; i < 256; i++) {
        if (src->model[i] == NULL)
            dst->model[i] = NULL;
        else {
            dst->model[i] = (unsigned char *) malloc(dst->model_size);
            memcpy(dst->model[i], src->model[i], dst->model_size);
        }
    }

    dst->plaintext_length = src->plaintext_length;
    dst->pow2div = src->pow2div;
    dst->pow2mod = src->pow2mod;

    return dst;
}


char * markov_gen (_markov * markov, uint64_t seed)
{
    int i;
    uint64_t sum = seed;

    markov->plaintext[0] = markov->first_chars[sum & markov->pow2mod];
    sum >>= markov->pow2div;
    sum ^= seed;
    for (i = 1; i < markov->plaintext_length; i++) {
        markov->plaintext[i] = (char) markov->model[(int) markov->plaintext[i-1]][sum & markov->pow2mod];
        sum >>= markov->pow2div;
        sum ^= seed ^ (sum << 17);
    }
    markov->plaintext[i] = '\0';

    return markov->plaintext;
}
