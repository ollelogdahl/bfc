// Copyright (c) 2021 Olle Lögdahl
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdbool.h>

#define MAX_BRANCH_DEPTH 1000
#define BRANCH_HASH_SIZE 32

typedef struct {
    unsigned depth;
    unsigned widths[MAX_BRANCH_DEPTH];
} branch_t;
#define BRANCH_INIT { .depth=0 }

void branch_push(branch_t *b);
void branch_pop(branch_t *b);
bool branch_empty(branch_t *b);

char *branch_get(branch_t *b);
