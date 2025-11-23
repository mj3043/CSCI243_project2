// interp.c
// Main program: command-line handling and REPL for postfix interpreter
// Handles symbol table loading, input processing, comments, and final dump
// @author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L   // for clean compilation on queeg

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interp.h"
#include "parser.h"
#include "symtab.h"

/// Program entry point
/// @param argc number of command-line arguments (1 or 2 expected)
/// @param argv program name and optional symbol table filename
/// @return EXIT_SUCCESS on clean exit, EXIT_FAILURE on usage error
int main(int argc, char **argv)
{
    /* Validate command-line arguments */
    if (argc > 2) {
        fprintf(stderr, "Usage: interp [sym-table]\n");
        return EXIT_FAILURE;
    }

    /* Load symbol table: either from file or create empty one */
    if (argc == 2) {
        build_table(argv[1]);           // exits on error (per spec)
    } else {
        build_table(NULL);              // empty table
    }

    /* Print initial symbol table (only if non-empty) */
    dump_table();

    printf("Enter postfix expressions (CTRL-D to exit):\n");

    char linebuf[MAX_LINE + 2];         // +2 for '\n' and '\0'

    while (1) {
        printf("> ");
        fflush(stdout);

        /* Read one line; handle EOF gracefully */
        if (!fgets(linebuf, sizeof(linebuf), stdin)) {
            break;  // Ctrl-D or input error
        }

        /* Detect and reject overly long lines */
        size_t len = strlen(linebuf);
        if (len == sizeof(linebuf) - 1 && linebuf[len-1] != '\n') {
            fprintf(stderr, "Input line too long\n");
            int c;
            while ((c = getchar()) != EOF && c != '\n') ;  // discard rest
            continue;
        }

        /* Remove trailing newline */
        if (len > 0 && linebuf[len-1] == '\n') {
            linebuf[len-1] = '\0';
        }

        /* Skip full-line comments starting with # */
        char *p = linebuf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#') continue;

        /* Remove inline comments (everything after #) */
        char *hash = strchr(linebuf, '#');
        if (hash) *hash = '\0';

        /* Trim leading and trailing whitespace */
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

        /* Skip blank lines after trimming */
        if (*start == '\0') continue;

        /* Process the expression */
        rep(start);
    }

    /* Final symbol table dump on exit (only if non-empty) */
    dump_table();

    /* Clean up symbol table memory */
    free_table();

    return EXIT_SUCCESS;
}