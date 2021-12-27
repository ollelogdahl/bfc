/**
 * Copyright (c) 2021 Olle Lögdahl
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "asm.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <asm/unistd_64.h>

#define NL "\n"

void asm_header(asm_info_t *info, FILE *out) {
    // Writes the header
    // %rdi stores the data pointer. %rsp stores the stack pointer, and
    // %rbp stores the base pointer.
    fprintf(out,
    ".section .text" NL
    ".global _start" NL
    "_start:" NL
    "movq %%rsp, %%rax" NL
    "subq $%d, %%rsp" NL
    "movq %%rsp, %%rdi" NL
    "addq $%d, %%rdi" NL
    "_program:" NL
    , 2 * info->stack_size, info->stack_size);
}

void asm_footer(asm_info_t *info, FILE *out) {
    fprintf(out,
    "_end:" NL
    "movq $0, %%rdi" NL
    "movq $%d, %%rax" NL
    "syscall" NL
    , __NR_exit);
}

void asm_move(asm_info_t *info, FILE *out, const int offset) {
    char *op = (offset > 0) ? "addq" : "subq";
    fprintf(out, "%s $%d, %%rdi\n", op, abs(offset));
}

void asm_diff(asm_info_t *info, FILE *out, const int diff) {
    assert(diff != 0);

    char *op = (diff > 0) ? "addb" : "subb";
    fprintf(out, "%s $%d, (%%rdi)\n", op, abs(diff));
}

void asm_set(asm_info_t *info, FILE *out, const int value) {
    fprintf(out, "movb $%d, (%%rdi)\n", value);
}

void asm_copy(asm_info_t *info, FILE *out, const int offset) {
    fprintf(out,
    "movb (%%rdi), %%ah" NL
    "addb %%ah, %d(%%rdi)" NL
    , offset);
}

void asm_branch_begin(asm_info_t *info, FILE *out, const char *name) {
    fprintf(out,
    "movb (%%rdi), %%al" NL
    "cmpb $0, %%al" NL
    "je b_%s_end" NL
    "b_%s_start:" NL
    , name, name);
}

void asm_branch_end(asm_info_t *info, FILE *out, const char *name) {
    fprintf(out,
    "movb (%%rdi), %%al" NL
    "cmpb $0, %%al" NL
    "jne b_%s_start" NL
    "b_%s_end:" NL
    , name, name);
}

void asm_write(asm_info_t *info, FILE *out) {
    // calls write(STDOUT, %rdi, 1);
    fprintf(out,
    "pushq %%rdi" NL
    "movq %%rdi, %%rsi" NL
    "movq $1, %%rdx" NL
    "movq $%d, %%rdi" NL
    "movq $%d, %%rax" NL 
    "syscall" NL
    "popq %%rdi" NL
    , STDOUT_FILENO, __NR_write);
}

void asm_read(asm_info_t *info, FILE *out) {
    fprintf(out,
    "pushq %%rdi" NL
    "movq %%rdi, %%rsi" NL
    "movq $1, %%rdx" NL
    "movq $%d, %%rdi" NL
    "movq $%d, %%rax" NL
    "syscall" NL
    "popq %%rdi" NL
    , STDIN_FILENO, __NR_read);
}

void asm_comment(asm_info_t *info, FILE *out, char *fmt, ...) {
    va_list lst;
    va_start(lst, fmt);

    fprintf(out, "#");
    vfprintf(out, fmt, lst);
    fprintf(out, "\n");

    va_end(lst);
}
