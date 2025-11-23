// @author: tvf

#ifndef PARSER_H
#define PARSER_H

#include "tree_node.h"
#include "stack.h"

// The types of errors that can be run into while parsing
// or evaluating the tree
typedef enum parse_error_e {
    PARSE_NONE,                 // no problems
    TOO_FEW_TOKENS,             // not enough tokens in expression
    TOO_MANY_TOKENS,            // too many tokens in expression
    INVALID_ASSIGNMENT,         // assign to left hand side not a variable
    ILLEGAL_TOKEN               // doesn't fit any other pattern
} parse_error_t;

typedef enum eval_error_e {
    EVAL_NONE,
    DIVISION_BY_ZERO,
    INVALID_MODULUS,
    UNDEFINED_SYMBOL,
    UNKNOWN_OPERATION,
    UNKNOWN_EXP_TYPE,
    MISSING_LVALUE,
    INVALID_LVALUE,
    SYMTAB_FULL
} eval_error_t;

/// The main read-eval-print function that reads the expression,
/// parses it, and evaluates the result, printing the infix expression
/// and the resulting value to standard output.
/// process, using the rest of the routines defined here.
/// @param exp The expression as a string
void rep(char *exp);

/// Recursively build the parse tree from items on the stack
/// @param stack  the list of tokens to parse
/// @return the root of the parse tree, or NULL on failure
/// @exception will occur if the parse fails
tree_node_t *parse(stack_t *stack);

/// Constructs the expression tree from the expression.  It
/// must use the stack to order the tokens.  It must also
/// deallocate the memory associated with the stack in all cases.
/// If a symbol is encountered, it should be stored in the node
/// without checking if it is in the symbol table - evaluation will
/// resolve that issue.
/// @param expr the postfix expression as a C string
/// @return the root of the expression tree
/// @exception There are 2 error conditions that you must deal
///     with.  In each case, the memory associated with the
///     tree must be cleaned up before returning.  Neither 
///     stops execution of the program. 
///
///     1. If there are too few tokens, set the parser error
///     to TOO_FEW_TOKENS and display the message to standard error:
///
///     Invalid expression, not enough tokens
///
///     2. If there are many tokens (stack is not empty after building),
///     set the parser error to TOO_MANY_TOKENS and display the message
///     to standard error:
///
///     Invalid expression, too many tokens
tree_node_t *make_parse_tree(char *expr);

/// Evaluates the tree and returns the result.
/// @param node The node in the tree: either an INTERIOR or LEAF node
/// @precondition:  This routine should not be called if there
///     is a parser error.
/// @return the evaluated int.  Note:  A symbol evaluates
///     to the value bound to it.
int eval_tree(tree_node_t * node);

/// Displays the infix expression for the tree, using
/// parentheses to indicate the precedence, e.g.:
///
/// postfix expression: 10 20 + 30 *
/// infix string: ((10+20)*30) 
///
/// @param node  the tree_node of the tree to print
/// @precondition:  This routine should not be called if there
///     is a parser error.
void print_infix(tree_node_t * node);

/// Cleans up all dynamic memory associated with the expression tree.
/// @param node The current node in the tree
void cleanup_tree(tree_node_t * node);

#endif
