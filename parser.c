// parser.c
// Full postfix-expression parser and evaluator
// FINAL VERSION (cleaned + commented)
// Author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "tree_node.h"
#include "stack.h"
#include "symtab.h"

/* Global error indicators */
static parse_error_t parser_error = PARSE_NONE;
static eval_error_t evaluator_error = EVAL_NONE;

/* Custom strtok that treats whitespace as delimiter */
static char *my_strtok_r(char *str, const char *delim, char **saveptr)
{
    (void)delim;

    if (str)
        *saveptr = str;

    if (!*saveptr)
        return NULL;

    /* Skip leading whitespace */
    *saveptr += strspn(*saveptr, " \t\r\n");
    if (!**saveptr) {
        *saveptr = NULL;
        return NULL;
    }

    char *tok = *saveptr;

    /* Next whitespace ends token */
    *saveptr = strpbrk(tok, " \t\r\n");
    if (*saveptr) {
        **saveptr = '\0';
        (*saveptr)++;
    }

    return tok;
}

/* Error-setters */
static void set_parse_error(parse_error_t e, const char *m)
{
    parser_error = e;
    (void)m;
}

static void set_eval_error(eval_error_t e, const char *m)
{
    evaluator_error = e;
    (void)m;
}

/* Operator tests */
static int is_op(const char *t)
{
    return strcmp(t, "+") == 0 ||
           strcmp(t, "-") == 0 ||
           strcmp(t, "*") == 0 ||
           strcmp(t, "/") == 0 ||
           strcmp(t, "%") == 0 ||
           strcmp(t, "?") == 0 ||
           strcmp(t, "<-") == 0;
}

static op_type_t to_op(const char *t)
{
    if (strcmp(t, "+") == 0) return ADD_OP;
    if (strcmp(t, "-") == 0) return SUB_OP;
    if (strcmp(t, "*") == 0) return MUL_OP;
    if (strcmp(t, "/") == 0) return DIV_OP;
    if (strcmp(t, "%") == 0) return MOD_OP;
    if (strcmp(t, "<-") == 0) return ASSIGN_OP;
    return Q_OP;  // '?'
}

/* Integer validation */
static int is_int(const char *t)
{
    if (!t || !*t)
        return 0;

    int i = (*t == '-') ? 1 : 0;
    if (!isdigit((unsigned char)t[i]))
        return 0;

    for (; t[i]; i++)
        if (!isdigit((unsigned char)t[i]))
            return 0;

    return 1;
}

/* Symbol validation */
static int is_sym(const char *t)
{
    if (!t || !isalpha((unsigned char)*t))
        return 0;

    for (int i = 1; t[i]; i++)
        if (!isalnum((unsigned char)t[i]))
            return 0;

    return 1;
}

/* Recursive construction of parse tree from stack */
tree_node_t *parse(stack_t *s)
{
    if (!s || empty_stack(s)) {
        set_parse_error(TOO_FEW_TOKENS, NULL);
        return NULL;
    }

    char *tok = strdup((char *)top(s));
    pop(s);
    if (!tok) return NULL;

    /* Operator */
    if (is_op(tok)) {

        /* Ternary operator */
        if (strcmp(tok, "?") == 0) {
            tree_node_t *f = parse(s);
            tree_node_t *t = parse(s);
            tree_node_t *c = parse(s);

            if (parser_error) {
                free(tok);
                cleanup_tree(f);
                cleanup_tree(t);
                cleanup_tree(c);
                return NULL;
            }

            tree_node_t *alt = make_interior(ALT_OP, ":", t, f);
            tree_node_t *q   = make_interior(Q_OP, "?", c, alt);

            free(tok);
            return q;
        }

        /* Binary operators */
        tree_node_t *right = parse(s);
        tree_node_t *left  = parse(s);

        if (parser_error) {
            free(tok);
            cleanup_tree(left);
            cleanup_tree(right);
            return NULL;
        }

        tree_node_t *node = make_interior(to_op(tok), tok, left, right);
        free(tok);
        return node;
    }

    /* Leaf (integer or symbol) */
    tree_node_t *leaf =
        is_int(tok) ? make_leaf(INTEGER, tok) :
        is_sym(tok) ? make_leaf(SYMBOL,  tok) :
        NULL;

    if (!leaf)
        set_parse_error(ILLEGAL_TOKEN, NULL);

    free(tok);
    return leaf;
}

/* Tokenize and build parse tree */
tree_node_t *make_parse_tree(char *e)
{
    parser_error = PARSE_NONE;

    if (!e || !*e) {
        set_parse_error(TOO_FEW_TOKENS, NULL);
        return NULL;
    }

    stack_t *s = make_stack();
    char *copy = strdup(e);
    if (!copy) {
        free_stack(s);
        return NULL;
    }

    char *sp = NULL;
    char *t = my_strtok_r(copy, " \t\r\n", &sp);

    while (t) {
        char *d = strdup(t);
        if (!d) {
            free(copy);
            free_stack(s);
            set_parse_error(ILLEGAL_TOKEN, NULL);
            return NULL;
        }
        push(s, d);
        t = my_strtok_r(NULL, " \t\r\n", &sp);
    }

    free(copy);

    if (empty_stack(s)) {
        free_stack(s);
        set_parse_error(TOO_FEW_TOKENS, NULL);
        return NULL;
    }

    tree_node_t *root = parse(s);

    if (parser_error != PARSE_NONE) {
        if (root) cleanup_tree(root);
        free_stack(s);
        return NULL;
    }

    /* If items remain on stack → too many tokens */
    if (!empty_stack(s)) {
        cleanup_tree(root);
        free_stack(s);
        set_parse_error(TOO_MANY_TOKENS, NULL);
        return NULL;
    }

    /* Free stack nodes, but not tokens (now owned by tree) */
    while (s->top) {
        stack_node_t *node = s->top;
        s->top = node->next;
        free(node);
    }
    free(s);

    return root;
}

/* Evaluate parse tree */
int eval_tree(tree_node_t *n)
{
    evaluator_error = EVAL_NONE;

    if (!n) {
        set_eval_error(UNKNOWN_OPERATION, NULL);
        return 0;
    }

    /* Leaf */
    if (n->type == LEAF) {
        leaf_node_t *ln = (leaf_node_t *)n->node;

        if (ln->exp_type == INTEGER)
            return (int)strtol(n->token, NULL, 10);

        symbol_t *sym = lookup_table(n->token);
        if (!sym) {
            set_eval_error(UNDEFINED_SYMBOL, NULL);
            return 0;
        }
        return sym->val;
    }

    interior_node_t *in = (interior_node_t *)n->node;

    /* Assignment operator */
    if (in->op == ASSIGN_OP) {

        /* Left operand must be a symbol */
        if (in->left->type != LEAF ||
            ((leaf_node_t *)in->left->node)->exp_type != SYMBOL) {
            set_eval_error(INVALID_LVALUE, NULL);
            return 0;
        }

        char *name = in->left->token;

        /* Evaluate RHS */
        evaluator_error = EVAL_NONE;
        int val = eval_tree(in->right);

        if (evaluator_error == UNDEFINED_SYMBOL) {
            val = 0;
            evaluator_error = EVAL_NONE;
        } else if (evaluator_error != EVAL_NONE) {
            return 0;
        }

        symbol_t *s = lookup_table(name);
        if (!s) {
            s = create_symbol(name, val);
            if (!s) {
                set_eval_error(SYMTAB_FULL, NULL);
                return 0;
            }
        } else {
            s->val = val;
        }

        return val;
    }

    /* Ternary operator */
    if (in->op == Q_OP) {
        int cond = eval_tree(in->left);
        if (evaluator_error) return 0;

        interior_node_t *alt = (interior_node_t *)in->right->node;

        return cond ? eval_tree(alt->left)
                    : eval_tree(alt->right);
    }

    /* Binary operator */
    int left  = eval_tree(in->left);
    if (evaluator_error) return 0;

    int right = eval_tree(in->right);
    if (evaluator_error) return 0;

    switch (in->op) {
        case ADD_OP: return left + right;
        case SUB_OP: return left - right;
        case MUL_OP: return left * right;

        case DIV_OP:
            if (!right) {
                set_eval_error(DIVISION_BY_ZERO, NULL);
                return 0;
            }
            return left / right;

        case MOD_OP:
            if (!right) {
                set_eval_error(INVALID_MODULUS, NULL);
                return 0;
            }
            return left % right;

        default:
            set_eval_error(UNKNOWN_OPERATION, NULL);
            return 0;
    }
}

/* Infix pretty-printer */
void print_infix(tree_node_t *n)
{
    if (!n) return;

    if (n->type == LEAF) {
        printf("%s", n->token);
        return;
    }

    interior_node_t *in = (interior_node_t *)n->node;

    if (in->op == Q_OP) {
        printf("(");
        print_infix(in->left);

        printf("?");
        interior_node_t *alt = (interior_node_t *)in->right->node;

        printf("(");
        print_infix(alt->left);
        printf(":");
        print_infix(alt->right);
        printf(")");

        printf(")");
        return;
    }

    /* Assignment or binary operators */
    printf("(");
    print_infix(in->left);

    if (in->op == ASSIGN_OP)
        printf("=");
    else
        printf("%s", n->token);

    print_infix(in->right);
    printf(")");
}

/* Read–Eval–Print handler */
void rep(char *e)
{
    if (!e || !*e) return;

    parser_error   = PARSE_NONE;
    evaluator_error = EVAL_NONE;

    tree_node_t *root = make_parse_tree(e);
    if (!root || parser_error) {
        if (root) cleanup_tree(root);
        return;
    }

    int val = eval_tree(root);

    if (evaluator_error == EVAL_NONE) {
        print_infix(root);
        printf(" = %d\n", val);
    }

    cleanup_tree(root);
}
