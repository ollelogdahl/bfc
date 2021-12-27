// Copyright (c) 2021 Olle Lögdahl
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdio.h>

typedef enum {
    MOV,
    MOD,
    BRANCH,
    READ,
    WRITE
} tokentype_e;

typedef struct toklist_t toklist_t;

typedef struct {
    tokentype_e type;
    union {
        int i;
        char *n;
    };

    toklist_t *children;
} tok_t;

typedef struct toklist_t {
    unsigned capacity;
    unsigned count;
    tok_t **items;
} toklist_t;

#define TOKLIST_INIT { .capacity = 0, .count = 0, .items = NULL }

void tokenize(toklist_t *list, FILE *in);

void print_tok_tree(toklist_t *list, FILE *out);
