/**
 * Copyright (c) 2021 Olle Lögdahl
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "token.h"

#include <stdlib.h>
#include <string.h>

#include "err.h"

toklist_t *toklist_create(void);
void toklist_add(toklist_t *list, tok_t *tok);
tok_t *toklist_mk(toklist_t *list, tokentype_e type);
tok_t *toklist_mk_i(toklist_t *list, tokentype_e type, int i);
tok_t *toklist_mk_n(toklist_t *list, tokentype_e type, char *n);

#define IS_INSTR(ch) (c == '+' || c == '-' || c == '.' || c == ',' || c == '<' \
    || c == '>' || c == '[' || c == ']')
#define IS_MOV(ch) ((char) ch == '<' || (char) ch == '>')
#define IS_VAL(ch) ((char) ch == '-' || (char) ch == '+')

#define CURR_ROOT scoperoots[block_depth]
#define CURR_SCOPE scoperoots[block_depth]->children
#define MAX_DEPTH 1024

char *create_branch_name(tok_t *root, unsigned branch_no);

void tokenize(toklist_t *list, FILE *in) {
    int mov_accum = 0;
    int mod_accum = 0;

    tok_t root = {
        .n = "",
        .children = list,
    };

    unsigned block_depth = 0;
    tok_t *scoperoots[MAX_DEPTH] = {&root};
    int branches[MAX_DEPTH] = {0};

    int c;
    while((c = fgetc(in)) != EOF) {
        // If the character is outside the instruction set, skip it.
        if(!IS_INSTR((char) c)) continue;

        // if we have a move accumulation, and the character is not a
        // continuation of that, or has reached the end, we must
        // write it to the program.
        if(mov_accum != 0 && !IS_MOV(c)) {
            toklist_mk_i(CURR_SCOPE, MOV, mov_accum);
            mov_accum = 0;
        }

        // if we have a value accumulation, and the character is not a
        // continuation of that, or is the end, we must
        // write it to the program.
        if(mod_accum != 0 && !IS_VAL(c)) {
            toklist_mk_i(CURR_SCOPE, MOD, mod_accum);
            mod_accum = 0;
        }

        switch (c) {
        case '>':
            mov_accum++;
            break;
        case '<':
            mov_accum--;
            break;
        case '+':
            mod_accum++;
            break;
        case '-':
            mod_accum--;
            break;
        case '[': {
            char *branch_name = create_branch_name(CURR_ROOT, branches[block_depth]);
            tok_t *new_branch = toklist_mk_n(CURR_SCOPE, BRANCH, branch_name);

            new_branch->children = toklist_create();

            // increase the number of branches on the current level,
            // and add the new branch to the top of the stack.
            branches[block_depth]++;
            scoperoots[++block_depth] = new_branch;
        } break;
        case ']':
            if(block_depth == 0) error("unmatched brackets. aborting.");

            // remove from the branch stack. The memory is not freed as
            // the scoperoot (branch) is still a child of another.
            scoperoots[block_depth--] = NULL;
            break;
        case '.':
            toklist_mk(CURR_SCOPE, WRITE);
            break;
        case ',':
            toklist_mk(CURR_SCOPE, READ);
            break;
        }
    }

    // There may be remaining move and value accumulations, but
    // As we never access this data later we can optimize them out.
    // Finally, we must dump move and value accumulations if they exist.
    /*
    if(mov_accum != 0) {
        toklist_mk_i(CURR_SCOPE, MOV, mov_accum);
        mov_accum = 0;
    }
    
    if(mod_accum != 0) {
        toklist_mk_i(CURR_SCOPE, MOD, mod_accum);
        mod_accum = 0;
    }
    */
}

tok_t *toklist_mk(toklist_t *list, tokentype_e type) {
    tok_t *t = malloc(sizeof *t);
    if(t == NULL) sys_error("malloc token");
    t->type = type;
    t->children = NULL;
    t->n = NULL;
    toklist_add(list, t);

    return t;
}

tok_t *toklist_mk_i(toklist_t *list, tokentype_e type, int i) {
    tok_t *t = toklist_mk(list, type);
    t->i = i;

    return t;
}

tok_t *toklist_mk_n(toklist_t *list, tokentype_e type, char *n) {
    tok_t *t = toklist_mk(list, type);
    t->n = n;

    return t;
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





char *create_branch_name(tok_t *root, unsigned branch_no) {
    char *str = malloc(100 * sizeof *str);
    if(str == NULL) sys_error("malloc branch name");

    size_t root_len = strlen(root->n);
    memcpy(str, root->n, root_len);
    encode(str + root_len, 100 - root_len, branch_no);

    return str;
}

toklist_t *toklist_create(void) {
    toklist_t *list = malloc(sizeof *list);
    if(list == NULL) sys_error("malloc toklist");
    list->capacity = 0;
    list->count = 0;
    list->items = NULL;

    return list;
}

void toklist_add(toklist_t *list, tok_t *tok) {
    list->count++;

    if(list->count > list->capacity) {
        list->capacity = 32 + 2 * list->count;
        list->items = realloc(list->items, list->capacity * sizeof *list->items);
        if(list->items == NULL) sys_error("realloc toklist items");
    }

    list->items[list->count - 1] = tok;
}




void print_tok_tree_part(toklist_t *list, FILE *out, unsigned depth) {
    for(int i = 0; i < list->count; ++i) {
        tok_t *tok = list->items[i];

        const char *name;
        switch (tok->type) {
        case MOV: name = "MOV"; break;
        case MOD: name = "MOD"; break;
        case BRANCH: name = "BRANCH"; break;
        case READ: name = "READ"; break;
        case WRITE: name = "WRITE"; break;
        }

        if(tok->type == BRANCH) {
            printf("%*s%s: n='%s'\n", depth, "", name, tok->i, tok->n);
            print_tok_tree_part(tok->children, out, depth + 2);
        } else {
            printf("%*s%s: i=%d\n", depth, "", name, tok->i, tok->n);
        }
    }
}


void print_tok_tree(toklist_t *list, FILE *out) {
    print_tok_tree_part(list, out, 0);
}
