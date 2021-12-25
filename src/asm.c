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

#include "err.h"
#include "branch.h"

#define NL "\n"

#define IS_MOV(ch) ((char) ch == '<' || (char) ch == '>')
#define IS_VAL(ch) ((char) ch == '-' || (char) ch == '+')
#define IS_END(ch) (ch == EOF)

#define DBG_INFO(ai, out, ...) if(ai->debug) bf_comment(ai, out, __VA_ARGS__)

// The mode for normal execution
#define MODE_NORMAL 0

// The mode entered when a loop is known to never run, and therefore should
// be skipped.
#define MODE_SKIP_BRANCH 1

void bf_header(asm_info_t *info, FILE *out);
void bf_footer(asm_info_t *info, FILE *out);
void bf_move(asm_info_t *info, FILE *out, int offset);
void bf_value(asm_info_t *info, FILE *out, int diff);
void bf_write(asm_info_t *info, FILE *out);
void bf_read(asm_info_t *info, FILE *out);

void bf_branch_begin(asm_info_t *info, FILE *out, branch_t *b);
void bf_branch_end(asm_info_t *info, FILE *out, branch_t *b);

void bf_comment(asm_info_t *info, FILE *out, char *fmt, ...);

typedef struct {
    branch_t *branch;
    int mov_accum;
    int val_accum;
    bool skip_next_branch;
} m_normal_ctx_t;

typedef struct {
    unsigned branch_depth;
} m_skip_branch_ctx_t;

void mode_normal(asm_info_t *ai, m_normal_ctx_t *ctx, unsigned char *sptr, int c, FILE *out);
void mode_skip_branch(asm_info_t *ai, m_skip_branch_ctx_t *ctx, unsigned char *sptr, int c, FILE *out);

void bf_to_asm(asm_info_t *info, FILE *in, FILE *out) {

    branch_t branch_info = BRANCH_INIT;
    unsigned char state = MODE_NORMAL;

    // Skips the first branch if there are no other operations before it.
    m_normal_ctx_t normal_ctx = {
        .branch = &branch_info,
        .skip_next_branch = true,
        .mov_accum = 0,
        .val_accum = 0,
    };

    m_skip_branch_ctx_t skip_branch_ctx = {
        .branch_depth = 0
    };

    // Writes the header
    bf_header(info, out);

    while(true) {
        int c = fgetc(in);

        switch (state) {
        case MODE_NORMAL:
            mode_normal(info, &normal_ctx, &state, c, out);
            break;
        case MODE_SKIP_BRANCH:
            mode_skip_branch(info, &skip_branch_ctx, &state, c, out);
            break;
        default:
            error("runtime error; invalid mode");
        }

        if(IS_END(c)) break;
    }

    // Writes the footer
    bf_footer(info, out);
}

void mode_normal(asm_info_t *ai, m_normal_ctx_t *ctx, unsigned char *sptr, int c, FILE *out) {
    // if we have a move accumulation, and the character is not a
    // continuation of that, or is the end, we must output it to the
    // program.
    if(ctx->mov_accum != 0 && (!IS_MOV(c) || IS_END(c))) {
        DBG_INFO(ai, out, "move %d", ctx->mov_accum);

        bf_move(ai, out, ctx->mov_accum);
        ctx->mov_accum = 0;
        ctx->skip_next_branch = false;
    }

    // if we have a value accumulation, and the character is not a
    // continuation of that, or is the end, we must output it to the
    // program.
    if(ctx->val_accum != 0 && (!IS_VAL(c) || IS_END(c))) {
        DBG_INFO(ai, out, "value %d", ctx->val_accum);

        bf_value(ai, out, ctx->val_accum);
        ctx->val_accum = 0;
        ctx->skip_next_branch = false;
    }

    if(IS_END(c)) return;

    switch ((char) c) {
    case '>':
        ctx->mov_accum++;
        break;
    case '<':
        ctx->mov_accum--;
        break;
    case '+':
        ctx->val_accum++;
        break;
    case '-':
        ctx->val_accum--;
        break;
    case '[':
        if(ctx->skip_next_branch) {
            *sptr = MODE_SKIP_BRANCH;
            return;
        }

        bf_branch_begin(ai, out, ctx->branch);
        break;
    case ']':
        ctx->skip_next_branch = true;
        bf_branch_end(ai, out, ctx->branch);
        break;
    case '.':
        DBG_INFO(ai, out, "syscall write");
        bf_write(ai, out);
        ctx->skip_next_branch = false;
        break;
    case ',':
        DBG_INFO(ai, out, "syscall read");
        bf_read(ai, out);
        ctx->skip_next_branch = false;
        break;
    }
}

void mode_skip_branch(asm_info_t *ai, m_skip_branch_ctx_t *ctx, unsigned char *sptr, int c, FILE *out) {
    switch ((char) c) {
    case '[':
        ctx->branch_depth++;
        break;
    case ']':
        if(ctx->branch_depth == 0) {
            *sptr = MODE_NORMAL;
        } else {
            ctx->branch_depth--;
        }
        break;
    case EOF:
        error("mismatched brackets. aborting.");
    }
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

void bf_branch_begin(asm_info_t *info, FILE *out, branch_t *b) {

    branch_push(b);
    char *label = branch_get(b);

    if(info->debug) bf_comment(info, out, "[ %s", label);

    fprintf(out,
    "movb (%%rdi), %%al" NL
    "cmpb $0, %%al" NL
    "je b_%s_end" NL
    "b_%s_start:" NL
    , label, label);

    free(label);
}

void bf_branch_end(asm_info_t *info, FILE *out, branch_t *b) {
    
    if(branch_empty(b)) error("mismatched brackets. aborting.");

    char *label = branch_get(b);
    branch_pop(b);

    if(info->debug) bf_comment(info, out, "] %s", label);

    fprintf(out,
    "movb (%%rdi), %%al" NL
    "cmpb $0, %%al" NL
    "jne b_%s_start" NL
    "b_%s_end:" NL
    , label, label);

    free(label);
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

void bf_comment(asm_info_t *info, FILE *out, char *fmt, ...) {
    va_list lst;
    va_start(lst, fmt);

    fprintf(out, "#");
    vfprintf(out, fmt, lst);
    fprintf(out, "\n");

    va_end(lst);
}