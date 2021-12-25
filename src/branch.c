/**
 * Copyright (c) 2021 Olle Lögdahl
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "branch.h"

#include <stdio.h>
#include <stdlib.h>

#include "err.h"

void branch_push(branch_t *b) {
    b->depth++;
}

void branch_pop(branch_t *b) {
    b->depth--;
    b->widths[b->depth]++;
}

bool branch_empty(branch_t *b) {
    return b->depth == 0;
}

/**
 * All characters are allowed in identifiers!
 */
static const char encode_table[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '.', '_'
};

static unsigned encode(char *field, unsigned len, unsigned val) {
    unsigned n = val;
    unsigned i = 0;
    do {
        field[i++] = encode_table[n % 64];
        n = n >> 6;

        if(n > len) error("encode: no space");
    } while(n > 0);

    return i;
}

char *branch_get(branch_t *b) {

    char *str = malloc(sizeof *str * BRANCH_HASH_SIZE);
    if(str == NULL) sys_error("malloc branch string");

    unsigned str_off = 0;
    for(unsigned i = 0; i < b->depth; ++i) {
        unsigned width = b->widths[i];

        str_off += encode(&str[str_off], BRANCH_HASH_SIZE - str_off, width);
    }
    str[str_off] = '\0';

    return str;
}