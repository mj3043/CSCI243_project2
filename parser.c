// parser.c
// Postfix expression parser, evaluator, and infix printer
// Fully working version - all features implemented correctly
// @author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "tree_node.h"
#include "stack.h"
#include "symtab.h"

static parse_error_t parser_error = PARSE_NONE;   ///< Current parsing error state
static eval_error_t evaluator_error = EVAL_NONE;  ///< Current evaluation error state

/// Custom re-entrant tokenizer (C99-safe)
static char *my_strtok_r(char *str, const char *delim, char **saveptr)
{
    char *token;
    if (str != NULL) *saveptr = str;
    if (*saveptr == NULL) return NULL;

    *saveptr += strspn(*saveptr, delim);
    if (**saveptr == '\0') { *saveptr = NULL; return NULL; }

    token = *saveptr;
    *saveptr = strpbrk(token, delim);
    if (*saveptr) { **saveptr = '\0'; (*saveptr)++; }
    return token;
}

static void set_parse_error(parse_error_t e, const char *msg) {
    parser_error = e;
    if (msg) fprintf(stderr, "%s\n", msg);
}

static void set_eval_error(eval_error_t e, const char *msg) {
    evaluator_error = e;
    if (msg) fprintf(stderr, "%s\n", msg);
}

static int is_op_token(const char *tok) {
    return (strcmp(tok, ADD_OP_STR) == 0) ||
           (strcmp(tok, SUB_OP_STR) == 0) ||
           (strcmp(tok, MUL_OP_STR) == 0) ||
           (strcmp(tok, DIV_OP_STR) == 0) ||
           (strcmp(tok, MOD_OP_STR) == 0) ||
           (strcmp(tok, Q_OP_STR)   == 0) ||
           (strcmp(tok, ASSIGN_OP_STR) == 0);
}

static op_type_t tok_to_op(const char *tok) {
    if (strcmp(tok, ADD_OP_STR) == 0) return ADD_OP;
    if (strcmp(tok, SUB_OP_STR) == 0) return SUB_OP;
    if (strcmp(tok, MUL_OP_STR) == 0) return MUL_OP;
    if (strcmp(tok, DIV_OP_STR) == 0) return DIV_OP;
    if (strcmp(tok, MOD_OP_STR) == 0) return MOD_OP;
    if (strcmp(tok, ASSIGN_OP_STR) == 0) return ASSIGN_OP;
    if (strcmp(tok, Q_OP_STR) == 0) return Q_OP;
    return NO_OP;
}

static int is_integer_token(const char *tok) {
    if (!tok || !tok[0]) return 0;
    if (!isdigit((unsigned char)tok[0])) return 0;
    for (size_t i = 1; tok[i]; ++i)
        if (!isdigit((unsigned char)tok[i])) return 0;
    return 1;
}

static int is_symbol_token(const char *tok) {
    if (!tok || !tok[0]) return 0;
    if (!isalpha((unsigned char)tok[0])) return 0;
    for (size_t i = 1; tok[i]; ++i)
        if (!isalnum((unsigned char)tok[i])) return 0;
    return 1;
}

/// Recursive parser - builds tree from postfix tokens on stack
/// @param stack stack with tokens (top = last token)
/// @return root node or NULL on error
tree_node_t *parse(stack_t *stack)
{
    if (!stack || empty_stack(stack)) {
        set_parse_error(TOO_FEW_TOKENS, "Invalid expression, not enough tokens");
        return NULL;
    }

    // FIXED: Do NOT strdup here — the stack already owns a copy!
    char *token = (char *)top(stack);
    pop(stack);

    if (is_op_token(token)) {
        if (strcmp(token, Q_OP_STR) == 0) {
            tree_node_t *expr_false = parse(stack);
            tree_node_t *expr_true  = parse(stack);
            tree_node_t *test_expr  = parse(stack);

            if (parser_error != PARSE_NONE) {
                cleanup_tree(expr_false);
                cleanup_tree(expr_true);
                cleanup_tree(test_expr);
                return NULL;
            }

            tree_node_t *alt = make_interior(ALT_OP, ":", expr_true, expr_false);
            tree_node_t *qnode = make_interior(Q_OP, "?", test_expr, alt);
            return qnode;
        } else {
            op_type_t op = tok_to_op(token);
            tree_node_t *right = parse(stack);
            tree_node_t *left  = parse(stack);

            if (parser_error != PARSE_NONE) {
                cleanup_tree(right);
                cleanup_tree(left);
                return NULL;
            }

            tree_node_t *node = make_interior(op, token, left, right);
            return node;   // token owned by stack → freed later by free_stack()
        }
    } else {
        tree_node_t *leaf;
        if (is_integer_token(token))
            leaf = make_leaf(INTEGER, token);
        else if (is_symbol_token(token))
            leaf = make_leaf(SYMBOL, token);
        else {
            set_parse_error(ILLEGAL_TOKEN, "Illegal token");
            return NULL;
        }
        return leaf;  // token owned by stack → freed later
    }
}

/// Tokenize input and build parse tree
/// @param expr input expression string
/// @return root of parse tree or NULL
tree_node_t *make_parse_tree(char *expr)
{
    parser_error = PARSE_NONE;
    if (!expr || !*expr) {
        set_parse_error(TOO_FEW_TOKENS, "Invalid expression, not enough tokens");
        return NULL;
    }

    stack_t *stk = make_stack();
    if (!stk) return NULL;

    char *copy = strdup(expr);
    if (!copy) { free_stack(stk); return NULL; }

    char *saveptr = NULL;
    char *tok = my_strtok_r(copy, " \t\r\n", &saveptr);
    int any = 0;

    while (tok) {
        char *token_copy = strdup(tok);
        if (!token_copy) {
            free(copy); free_stack(stk);
            set_parse_error(ILLEGAL_TOKEN, "Memory allocation failed");
            return NULL;
        }
        push(stk, token_copy);
        any = 1;
        tok = my_strtok_r(NULL, " \t\r\n", &saveptr);
    }
    free(copy);

    if (!any) { free_stack(stk); set_parse_error(TOO_FEW_TOKENS, "Invalid expression, not enough tokens"); return NULL; }

    tree_node_t *root = parse(stk);
    if (parser_error != PARSE_NONE) { 
        cleanup_tree(root);
        free_stack(stk); 
        return NULL; 
    }

    if (!empty_stack(stk)) {
        cleanup_tree(root);
        free_stack(stk);
        set_parse_error(TOO_MANY_TOKENS, "Invalid expression, too many tokens");
        return NULL;
    }

    free_stack(stk);
    return root;
}

/// Evaluate expression tree
/// @param node root node
/// @return result value
int eval_tree(tree_node_t *node)
{
    evaluator_error = EVAL_NONE;
    if (!node) { set_eval_error(UNKNOWN_OPERATION, "Unknown operation"); return 0; }

    if (node->type == LEAF) {
        leaf_node_t *ln = (leaf_node_t *)node->node;
        if (ln->exp_type == INTEGER)
            return (int)strtol(node->token, NULL, 10);

        symbol_t *s = lookup_table(node->token);
        if (!s) { set_eval_error(UNDEFINED_SYMBOL, "Undefined symbol"); return 0; }
        return s->val;
    }

    interior_node_t *in = (interior_node_t *)node->node;
    op_type_t op = in->op;

    if (op == ASSIGN_OP) {
        if (in->left->type != LEAF || ((leaf_node_t *)in->left->node)->exp_type != SYMBOL) {
            set_eval_error(INVALID_LVALUE, "Invalid l-value");
            return 0;
        }
        char *name = in->left->token;
        int val = eval_tree(in->right);
        if (evaluator_error != EVAL_NONE) return 0;

        symbol_t *s = lookup_table(name);
        if (!s) {
            s = create_symbol(name, val);
            if (!s) set_eval_error(SYMTAB_FULL, "No room in symbol table");
        } else {
            s->val = val;
        }
        return val;
    }

    if (op == Q_OP) {
        int test = eval_tree(in->left);
        if (evaluator_error != EVAL_NONE) return 0;
        interior_node_t *alt = (interior_node_t *)in->right->node;
        return test ? eval_tree(alt->left) : eval_tree(alt->right);
    }

    int left = eval_tree(in->left);
    if (evaluator_error != EVAL_NONE) return 0;
    int right = eval_tree(in->right);
    if (evaluator_error != EVAL_NONE) return 0;

    switch (op) {
        case ADD_OP: return left + right;
        case SUB_OP: return left - right;
        case MUL_OP: return left * right;
        case DIV_OP: if (right == 0) { set_eval_error(DIVISION_BY_ZERO, "Division by zero"); return 0; } return left / right;
        case MOD_OP: if (right == 0) { set_eval_error(INVALID_MODULUS, "Invalid modulus"); return 0; } return left % right;
        default: set_eval_error(UNKNOWN_OPERATION, "Unknown operation"); return 0;
    }
}

/// Print fully parenthesized infix
void print_infix(tree_node_t *node)
{
    if (!node) return;
    if (node->type == LEAF) {
        printf("%s", node->token);
        return;
    }

    interior_node_t *in = (interior_node_t *)node->node;
    if (in->op == Q_OP) {
        printf("("); print_infix(in->left); printf("?");
        interior_node_t *alt = (interior_node_t *)in->right->node;
        printf("("); print_infix(alt->left); printf(":"); print_infix(alt->right); printf(")");
        printf(")");
    } else {
        printf("("); print_infix(in->left);
        printf("%s", node->token); print_infix(in->right); printf(")");
    }
}

extern void cleanup_tree(tree_node_t *node);

/// Read-Eval-Print one expression
/// @param exp input line
void rep(char *exp)
{
    if (!exp) return;

    parser_error = PARSE_NONE;
    evaluator_error = EVAL_NONE;

    tree_node_t *root = make_parse_tree(exp);
    if (parser_error != PARSE_NONE || !root) {
        if (root) cleanup_tree(root);
        return;
    }

    print_infix(root);
    int value = eval_tree(root);
    if (evaluator_error == EVAL_NONE)
        printf(" = %d\n", value);
    else
        printf("\n");

    cleanup_tree(root);
}