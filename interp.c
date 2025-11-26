// interp.c
// Main program: command-line handling and REPL for postfix interpreter
// @author: Munkh-Orgil Jargalsaikhan

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
    /* Validate command-line arguments */
    if (argc > 2) {
        fprintf(stderr, "Usage: interp [sym-table]\n");
        return EXIT_FAILURE;
    }

    /* Load symbol table */
    if (argc == 2) {
        build_table(argv[1]);
    } else {
        build_table(NULL);
    }

    /* Initial dump */
    dump_table();

    char linebuf[MAX_LINE + 2];

    while (1) {
        if (!fgets(linebuf, sizeof(linebuf), stdin)) {
            break;  // EOF
        }

        size_t len = strlen(linebuf);
        if (len == sizeof(linebuf) - 1 && linebuf[len-1] != '\n') {
            fprintf(stderr, "Input line too long\n");
            int c;
            while ((c = getchar()) != EOF && c != '\n') ;
            continue;
        }

        if (len > 0 && linebuf[len-1] == '\n') {
            linebuf[len-1] = '\0';
        }
        char *cr = strchr(linebuf, '\r');
        if (cr) *cr = '\0';

        /* Skip full-line comments */
        char *p = linebuf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#') continue;

        /* Remove inline comments */
        char *hash = strchr(linebuf, '#');
        if (hash) *hash = '\0';

        /* Trim whitespace */
        char *start = linebuf;
        while (*start && isspace((unsigned char)*start)) start++;

        char *end = start + strlen(start);
        if (end != start) {
            end--;
            while (end >= start && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
            }
        }

        if (*start == '\0') continue;

        rep(start);
    }

    /* Final dump — no extra newline */
    dump_table();

    free_table();
    return EXIT_SUCCESS;
}