// Copyright (c) 2021 Olle Lögdahl
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int stack_size;
    bool debug;
} asm_info_t;

/**
 * Reads all characters from in, and processes them into
 * assembly. Writes the processed assembly to out.
 * @param info Information about how to assemble
 * @param in The file to read bf code from
 * @param out The file to write assembly to
 */
void bf_to_asm(asm_info_t *info, FILE *in, FILE *out);