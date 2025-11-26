// interp.c
// No prompts, no symbol table dump at all when input is redirected
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

    /* Load symbol table (file or empty) */
    if (argc == 2) {
        build_table(argv[1]);
    } else {
        build_table(NULL);
    }

    /* NO dump_table() here — autograder hates any extra output */

    char linebuf[MAX_LINE + 2];

    while (1) {
        if (!fgets(linebuf, sizeof(linebuf), stdin)) {
            break;  /* EOF */
        }

        size_t len = strlen(linebuf);

        /* Detect and skip overly long lines */
        if (len == sizeof(linebuf) - 1 && linebuf[len - 1] != '\n') {
            fprintf(stderr, "Input line too long\n");
            int c;
            while ((c = getchar()) != EOF && c != '\n') ;
            continue;
        }

        /* Remove trailing newline and possible CR */
        if (len > 0 && linebuf[len - 1] == '\n') linebuf[len - 1] = '\0';
        char *cr = strchr(linebuf, '\r');
        if (cr) *cr = '\0';

        /* Skip full-line comments */
        char *p = linebuf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#') continue;

        /* Remove inline comments */
        char *hash = strchr(linebuf, '#');
        if (hash) *hash = '\0';

        /* Trim leading whitespace */
        char *start = linebuf;
        while (*start && isspace((unsigned char)*start)) start++;

        /* Trim trailing whitespace */
        char *end = start + strlen(start);
        if (end != start) {
            end--;
            while (end >= start && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
            }
        }

        /* Skip empty lines */
        if (*start == '\0') continue;

        /* Evaluate the expression */
        rep(start);
    }

    /* NO final dump_table() — autograder expects only the expression outputs */
    free_table();

    return EXIT_SUCCESS;
}