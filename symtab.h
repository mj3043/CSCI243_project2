/// @author tvf

#ifndef SYMTAB_H
#define SYMTAB_H

#define BUFLEN 1024             // input buffer length for initial symbols

// A single symbol definition
typedef struct symbol_s {
    char *var_name;             // the name of the symbol
    int val;                    // the value currently bound to this symbol
    struct symbol_s *next;      // the next item in the list
} symbol_t;

/// Constructs the table by reading the file.  The format is
/// one symbol per line in the format:
///
///     variable-type variable-name     variable-value
///     ...
///
/// For this problem we guarantee the variable-names are all
/// unique, and the format of the file is error free.
/// You are allowed to create it statically or
/// dynamically and store it locally.
/// @param filename The name of the file containing the symbols
/// @exception If the file can't be opened, an error message should
/// be displayed to standard error and the program should exit
/// with EXIT_FAILURE.  If no file is specified, an empty table
/// should be built (there is no message or error/exit in this case).
///
/// Error loading symbol table
///
void build_table(char *filename);

/// Displays the contents of the symbol table in the following format:
///
/// SYMBOL TABLE:
///     Name: variable-name, Value: variable-value
///     ...
///
/// Each symbol should be printed one per line, tab-indented.
void dump_table(void);

/// Returns the symtab_t object in the symbol table associated
///     with the variable name
/// @param variable The name of the variable (a C string)
/// @return The symbol_t object containing the binding,
///     or NULL if not found
symbol_t *lookup_table(char *variable);

/// Adds a binding to the symbol table
/// @param name  The name of the variable (a C string)
/// @param val  The value associated with the variable
/// @return the new symbol_t object added to the table,
///     or NULL if no space is available
/// No check is done to see if the symbol is already in the table
symbol_t *create_symbol(char *name, int val);

/// Destroys the symbol table
void free_table(void);

#endif
