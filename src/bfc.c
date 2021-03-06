/**
 * Copyright (c) 2021 Olle Lögdahl
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "token.h"
#include "parser.h"
#include "err.h"

#define VERSION "0.1.1"

void help(FILE *fp, char *const cmd);
void version(FILE *fp);

pid_t spawn_parser(asm_info_t *info, FILE *in, int *pipe);
pid_t spawn_assembler(asm_info_t *info, int in_fd, const char *outfile);

#include "token.h"

int main(const int argc, char *const *argv) {

    char *outfile = "a.out";
    bool assemble = true;
    bool debug = false;

    int opt;
    while((opt = getopt(argc, argv, "hvgSo:")) != -1) {
        switch ((char) opt) {
        case 'o':
            outfile = optarg;
            break;
        case 'S':
            assemble = false;
            break;
        case 'g':
            debug = true;
            break;

        case 'h':
            help(stdout, argv[0]);
            exit(EXIT_SUCCESS);
        case 'v':
            version(stdout);
            exit(EXIT_SUCCESS);
        default:
            help(stdout, argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    char *infile = argv[optind];

    if(argc > optind + 1) {
        fprintf(stderr, "too many arguments: one expected\n");
        help(stderr, argv[0]);
        exit(EXIT_FAILURE);
    }
    if(argc <= optind) {
        fprintf(stderr, "expected a file to compile\n");
        help(stderr, argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(infile, "r");
    if(fp == NULL) sys_error("fopen '%s' failed", infile);

    // peek at the first character and see if the file is empty.
    int c = fgetc(fp);
    if(c == EOF) error("no input to compile, exiting.");
    ungetc(c, fp);

    asm_info_t info = {
        .stack_size = 30000,
        .debug = debug,
        .eof_handling = ZERO
    };

    if(assemble) {
        int p[2];
        if(pipe(p) == -1) sys_error("pipe");

        pid_t pid_parser = spawn_parser(&info, fp, p);
        pid_t pid_asm = spawn_assembler(&info, p[0], outfile);

        int parser_rc, asm_rc;
        if(waitpid(pid_parser, &parser_rc, 0) == -1) sys_error("wait parser");
        if(waitpid(pid_asm, &asm_rc, 0) == -1) sys_error("wait assembler");
    } else {
        FILE *out = fopen(outfile, "w");
        if(out == NULL) sys_error("fopen %s", outfile);
        
        toklist_t tokens = TOKLIST_INIT;
        tokenize(&tokens, fp);
        fclose(fp);

        parse(&info, &tokens, out);
        toklist_free(&tokens);
    }

    return 0;
}

pid_t spawn_parser(asm_info_t *info, FILE *in, int *pipe) {
    pid_t pid = fork();
    switch (pid) {
    case -1: sys_error("fork parser");
    case 0:
        close(pipe[0]);
        FILE *out = fdopen(pipe[1], "w");
        if(out == NULL) sys_error("fdopen pipe write-end");

        // tokenize
        toklist_t tokens = TOKLIST_INIT;
        tokenize(&tokens, in);
        fclose(in);

        parse(info, &tokens, out);
        toklist_free(&tokens);
        fclose(out);

        exit(EXIT_SUCCESS);
    }

    fclose(in);

    close(pipe[1]);
    return pid;
}

pid_t spawn_assembler(asm_info_t *info, int in_fd, const char *outfile) {
    pid_t pid = fork();
    switch (pid) {
    case -1: sys_error("fork assembler");
    case 0:
        dup2(in_fd, STDIN_FILENO);
        execlp("as", "--64", "-g", "-o", outfile, NULL);
        sys_error("execlp assembler");
    }
    
    close(in_fd);
    return pid;
}

void help(FILE *fp, char *const cmd) {
    fprintf(fp, "usage: %s [-gShv] [-o out_file] file\n"
        "  S        skips the assembler and only outputs the generated assembly.\n"
        "  g        generates assembly with comments about the bf instructions.\n"
        "  o file   the file to write the assembly/linkable object to.\n"
        "  h        shows this.\n"
        "  v        shows the version of the program.\n\n"
        "Report bugs to: olle@logdahl.net\n"
    , cmd);
}

void version(FILE *fp) {
    fprintf(fp, "bfc version %s\n"
        "Copyright (C) 2021 Olle Lögdahl.\n"
    , VERSION);
}