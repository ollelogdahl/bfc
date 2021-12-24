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
#include <unistd.h>
#include <asm/unistd_64.h>

#include "err.h"

#define NL "\n"

#define IS_MOV(ch) ((char) ch == '<' || (char) ch == '>')
#define IS_VAL(ch) ((char) ch == '-' || (char) ch == '+')
#define IS_END(ch) (ch == EOF)

void bf_header(asm_info_t *info, FILE *out);
void bf_footer(asm_info_t *info, FILE *out);
void bf_move(asm_info_t *info, FILE *out, int offset);
void bf_value(asm_info_t *info, FILE *out, int diff);
void bf_write(asm_info_t *info, FILE *out);
void bf_read(asm_info_t *info, FILE *out);
void bf_comment(asm_info_t *info, FILE *out, char *fmt, ...);

void bf_to_asm(asm_info_t *info, FILE *in, FILE *out) {

    // Writes the header
    bf_header(info, out);


    int mov_accum = 0;
    int val_accum = 0;

    while(true) {
        int c = fgetc(in);

        // if we have a move accumulation, and the character is not a
        // continuation of that, or is the end, we must output it to the
        // program.
        if(mov_accum != 0 && (!IS_MOV(c) || IS_END(c))) {
            if(info->debug) bf_comment(info, out, "dumping move %d", mov_accum);

            bf_move(info, out, mov_accum);
            mov_accum = 0;
        }

        // if we have a value accumulation, and the character is not a
        // continuation of that, or is the end, we must output it to the
        // program.
        if(val_accum != 0 && (!IS_VAL(c) || IS_END(c))) {
            if(info->debug) bf_comment(info, out, "dumping value %d", val_accum);

            bf_value(info, out, val_accum);
            val_accum = 0;
        }

        // Exit parsing if we have reached eof.
        if(IS_END(c)) break;

        switch ((char) c) {
        case '>':
            if(info->debug) bf_comment(info, out, ">");
            mov_accum++;
            break;
        case '<':
            if(info->debug) bf_comment(info, out, "<");
            mov_accum--;
            break;
        case '+':
            if(info->debug) bf_comment(info, out, "+");
            val_accum++;
            break;
        case '-':
            if(info->debug) bf_comment(info, out, "-");
            val_accum--;
            break;
        case '.':
            if(info->debug) bf_comment(info, out, ".");
            bf_write(info, out);
            break;
        case ',':
            if(info->debug) bf_comment(info, out, ",");
            bf_read(info, out);
            break;
        default:
            // ignore
            continue;
        }
    }

    // TEMP: Assert that accumulations are empty.
    assert(mov_accum == 0);
    assert(val_accum == 0);

    // Writes the footer
    bf_footer(info, out);
}

void bf_header(asm_info_t *info, FILE *out) {
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

void bf_footer(asm_info_t *info, FILE *out) {
    fprintf(out,
    "_end:" NL
    "movq $0, %%rdi" NL
    "movq $%d, %%rax" NL
    "syscall" NL
    , __NR_exit);
}

void bf_move(asm_info_t *info, FILE *out, int offset) {
    char *op = (offset > 0) ? "addq" : "subq";
    fprintf(out, "%s $%d, %%rdi\n", op, abs(offset));
}

void bf_value(asm_info_t *info, FILE *out, int diff) {
    char *op = (diff > 0) ? "addb" : "subb";
    fprintf(out, "%s $%d, (%%rdi)\n", op, abs(diff));
}

void bf_write(asm_info_t *info, FILE *out) {
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

void bf_read(asm_info_t *info, FILE *out) {
    assert(false);
}

void bf_comment(asm_info_t *info, FILE *out, char *fmt, ...) {
    va_list lst;
    va_start(lst, fmt);

    fprintf(out, "#");
    vfprintf(out, fmt, lst);
    fprintf(out, "\n");

    va_end(lst);
}