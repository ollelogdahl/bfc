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
    enum {
        ZERO,
        NEGATIVE,
        UNCHANGED
    } eof_handling;
} asm_info_t;

void asm_move(asm_info_t *info, FILE *out, const int offset);
void asm_diff(asm_info_t *info, FILE *out, const int diff);
void asm_set(asm_info_t *info, FILE *out, const int value);

/**
 * Performs a copy of the value at the current cell into another cell with
 * offset.
 */
void asm_copy(asm_info_t *info, FILE *out, const int offset);

/**
 * performs a multiplication to the current cell value with factor and
 * stores the result in a cell with offset.
 */
void asm_mult(asm_info_t *info, FILE *out, const int factor, const int offset);
void asm_header(asm_info_t *info, FILE *out);
void asm_footer(asm_info_t *info, FILE *out);
void asm_write(asm_info_t *info, FILE *out);
void asm_read(asm_info_t *info, FILE *out);
void asm_branch_begin(asm_info_t *info, FILE *out, const char *name);
void asm_branch_end(asm_info_t *info, FILE *out, const char *name);

void asm_comment(asm_info_t *info, FILE *out, char *fmt, ...);
