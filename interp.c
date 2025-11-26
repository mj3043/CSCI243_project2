// interp.c
// Main program: command-line handling and REPL for postfix interpreter
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
    /* Validate command-line arguments */
    if (argc > 2) {
        fprintf(stderr, "Usage: interp [sym-table]\n");
        return EXIT_FAILURE;
    }

    /* Load symbol table file or empty table */
    if (argc == 2) {
        build_table(argv[1]);
    } else {
        build_table(NULL);
    }

    /* Initial dump before processing input */
    dump_table();

    char linebuf[MAX_LINE + 2];

    while (1) {

        /* Read a single line */
        if (!fgets(linebuf, sizeof(linebuf), stdin)) {
            break;  // EOF
        }

        /* Detect overly long line */
        size_t len = strlen(linebuf);
        if (len == sizeof(linebuf) - 1 && linebuf[len - 1] != '\n') {
            fprintf(stderr, "Input line too long\n");

            int c;
            while ((c = getchar()) != EOF && c != '\n') ;
            continue;
        }

        /* Strip trailing newline */
        if (len > 0 && linebuf[len - 1] == '\n') {
            linebuf[len - 1] = '\0';
        }

        /* Strip CR if present (Windows input) */
        char *cr = strchr(linebuf, '\r');
        if (cr) *cr = '\0';

        /* Skip full-line comments */
        char *p = linebuf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#') continue;

        /* Remove inline comments beginning with '#' */
        char *hash = strchr(linebuf, '#');
        if (hash) *hash = '\0';

        /* Leading trim */
        char *start = linebuf;
        while (*start && isspace((unsigned char)*start)) start++;

        /* Trailing trim */
        char *end = start + strlen(start);
        if (end != start) {
            end--;
            while (end >= start && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
            }
        }

        /* Skip blank line */
        if (*start == '\0') continue;

        /* Evaluate */
        rep(start);
    }

    /* Final dump without extra newline */
    dump_table();
    free_table();

    return EXIT_SUCCESS;
}
