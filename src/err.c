/**
 * Copyright (c) 2021 Olle Lögdahl
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

noreturn void error(char *fmt, ...) {
    va_list lst;
    va_start(lst, fmt);

    // fprintf(stderr, RED "ERR: " NORM "\n");
    vfprintf(stderr, fmt, lst);
    fprintf(stderr, "\n");

    va_end(lst);
    exit(EXIT_FAILURE);
}

noreturn void sys_error(char *fmt, ...) {
    va_list lst;
    va_start(lst, fmt);

    // fprintf(stderr, RED "SYSTEM ERR: " NORM "\n");
    vfprintf(stderr, fmt, lst);
    fprintf(stderr, ": %s\n", strerror(errno));

    va_end(lst);
    exit(EXIT_FAILURE);
}