// symtab.c
// Simple linked-list symbol table with load-from-file support
// @author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L   // for strdup() under strict C99

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/// Head of the symbol table linked list (most recently added first)
static symbol_t *sym_head = NULL;


/// Load symbol table from file (or create empty table if filename is NULL)
/// Each valid line must be: <name> <integer_value>
/// Lines starting with # or empty lines are ignored
/// Malformed lines cause immediate exit with error message
/// @param filename path to symbol file, or NULL for empty table
void build_table(char *filename)
{
    if (!filename) {
        sym_head = NULL;  // ensure empty table
        return;
    }

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    char buf[BUFLEN];
    while (fgets(buf, BUFLEN, f)) {
        char *p = buf;

        // skip leading whitespace
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\0' || *p == '\n') continue;

        char name[BUFLEN];
        int value;
        int matched = sscanf(buf, " %1023s %d", name, &value);

        if (matched == 2) {
            create_symbol(name, value);  // ignore return value as per spec
        } else {
            fprintf(stderr, "Error loading symbol table: malformed line\n");
            fclose(f);
            exit(EXIT_FAILURE);
        }
    }
    fclose(f);
}


/// Print entire symbol table in required format
/// Only prints if table is non-empty
void dump_table(void)
{
    if (!sym_head) return;

    printf("SYMBOL TABLE:\n");
    for (symbol_t *cur = sym_head; cur != NULL; cur = cur->next) {
        printf("\tName: %s, Value: %d\n", cur->var_name, cur->val);
    }
}


/// Search symbol table for variable name
/// @param variable name to look up
/// @return pointer to symbol if found, NULL otherwise
symbol_t *lookup_table(char *variable)
{
    if (!variable) return NULL;

    for (symbol_t *cur = sym_head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->var_name, variable) == 0) {
            return cur;
        }
    }
    return NULL;
}


/// Create a new symbol and insert at head of list
/// @param name variable name (will be duplicated)
/// @param val initial integer value
/// @return pointer to new symbol, or NULL on allocation failure
symbol_t *create_symbol(char *name, int val)
{
    if (!name) return NULL;

    symbol_t *new_sym = malloc(sizeof(symbol_t));
    if (!new_sym) {
        perror("malloc");
        return NULL;
    }

    new_sym->var_name = strdup(name);
    if (!new_sym->var_name) {
        perror("strdup");
        free(new_sym);
        return NULL;
    }

    new_sym->val = val;
    new_sym->next = sym_head;
    sym_head = new_sym;

    return new_sym;
}


/// Free all memory used by the symbol table
void free_table(void)
{
    symbol_t *cur = sym_head;
    while (cur != NULL) {
        symbol_t *next = cur->next;
        if (cur->var_name) free(cur->var_name);
        free(cur);
        cur = next;
    }
    sym_head = NULL;
}