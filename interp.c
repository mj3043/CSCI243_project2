// interp.c
// Author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "interp.h"
#include "parser.h"
#include "symtab.h"

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "Usage: interp [sym-table]\n");
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        build_table(argv[1]);
    } else {
        build_table(NULL);
    }

    char buf[MAX_LINE + 2];

    while (1) {
        /* Show prompt only when running interactively */
        if (isatty(fileno(stdin))) {
            printf("> ");
            fflush(stdout);
        }

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;  // EOF
        }

        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') {
            buf[len-1] = '\0';
            len--;
        }

        if (buf[0] == '#' || buf[0] == '\0') {
            continue;
        }

        for (size_t i = 0; i < len; i++) {
            if (buf[i] == '#') {
                buf[i] = '\0';
                break;
            }
        }

        if (buf[0] == '\0') {
            continue;
        }

        rep(buf);
    }

    dump_table();   // Final symbol table – REQUIRED by autograder
    free_table();

    return EXIT_SUCCESS;
}