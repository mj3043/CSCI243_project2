// parser.c
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

static parse_error_t parser_error = PARSE_NONE;
static eval_error_t   eval_error   = EVAL_NONE;

static char *my_strtok_r(char *str, const char *delim, char **saveptr)
{
    (void)delim;
    if (str) *saveptr = str;
    if (!*saveptr) return NULL;
    *saveptr += strspn(*saveptr, " \t\r\n");
    if (!**saveptr) { *saveptr = NULL; return NULL; }
    char *tok = *saveptr;
    *saveptr = strpbrk(tok, " \t\r\n");
    if (*saveptr) { **saveptr = '\0'; (*saveptr)++; }
    return tok;
}

static int is_op(const char *t)
{
    return strcmp(t,"+")==0 || strcmp(t,"-")==0 || strcmp(t,"*")==0 ||
           strcmp(t,"/")==0 || strcmp(t,"%")==0 || strcmp(t,"?")==0 || strcmp(t,"<-")==0;
}

static op_type_t to_op(const char *t)
{
    if (!strcmp(t,"+"))  return ADD_OP;
    if (!strcmp(t,"-"))  return SUB_OP;
    if (!strcmp(t,"*"))  return MUL_OP;
    if (!strcmp(t,"/"))  return DIV_OP;
    if (!strcmp(t,"%"))  return MOD_OP;
    if (!strcmp(t,"<-")) return ASSIGN_OP;
    return Q_OP;
}

static int is_int(const char *t)
{
    if (!t || !*t) return 0;
    int i = (*t == '-') ? 1 : 0;
    if (!isdigit((unsigned char)t[i])) return 0;
    for (; t[i]; i++) if (!isdigit((unsigned char)t[i])) return 0;
    return 1;
}

static int is_sym(const char *t)
{
    if (!t || !isalpha((unsigned char)*t)) return 0;
    for (int i = 1; t[i]; i++) if (!isalnum((unsigned char)t[i])) return 0;
    return 1;
}

tree_node_t *parse(stack_t *s)
{
    if (!s || empty_stack(s)) { parser_error = TOO_FEW_TOKENS; return NULL; }

    const char *src = top(s); pop(s);
    char *tok = strdup(src);
    if (!tok) return NULL;

    if (is_op(tok)) {
        if (!strcmp(tok,"?")) {
            tree_node_t *f = parse(s);
            tree_node_t *t = parse(s);
            tree_node_t *c = parse(s);
            if (parser_error) { free(tok); cleanup_tree(f); cleanup_tree(t); cleanup_tree(c); return NULL; }
            tree_node_t *alt = make_interior(ALT_OP, ":", t, f);
            tree_node_t *q   = make_interior(Q_OP, "?", c, alt);
            free(tok);
            return q;
        }
        tree_node_t *right = parse(s);
        tree_node_t *left  = parse(s);
        if (parser_error) { free(tok); cleanup_tree(left); cleanup_tree(right); return NULL; }
        tree_node_t *n = make_interior(to_op(tok), tok, left, right);
        free(tok);
        return n;
    }

    tree_node_t *leaf = is_int(tok) ? make_leaf(INTEGER, tok) :
                        is_sym(tok) ? make_leaf(SYMBOL, tok) : NULL;
    if (!leaf) { parser_error = ILLEGAL_TOKEN; free(tok); return NULL; }
    free(tok);
    return leaf;
}

tree_node_t *make_parse_tree(char *e)
{
    parser_error = PARSE_NONE;
    if (!e || !*e) return NULL;

    stack_t *s = make_stack();
    char *copy = strdup(e);
    if (!copy) { free_stack(s); return NULL; }

    char *sp = NULL;
    char *t = my_strtok_r(copy, " \t\r\n", &sp);
    while (t) {
        push(s, t);
        t = my_strtok_r(NULL, " \t\r\n", &sp);
    }
    free(copy);

    if (empty_stack(s)) { free_stack(s); return NULL; }

    tree_node_t *root = parse(s);
    if (parser_error) { if (root) cleanup_tree(root); free_stack(s); return NULL; }
    if (!empty_stack(s)) { cleanup_tree(root); free_stack(s); parser_error = TOO_MANY_TOKENS; return NULL; }

    while (s->top) { stack_node_t *n = s->top; s->top = n->next; free(n); }
    free(s);
    return root;
}

int eval_tree(tree_node_t *n)
{
    eval_error = EVAL_NONE;
    if (!n) { eval_error = UNKNOWN_OPERATION; return 0; }

    if (n->type == LEAF) {
        leaf_node_t *ln = (leaf_node_t *)n->node;
        if (ln->exp_type == INTEGER)
            return (int)strtol(n->token, NULL, 10);
        symbol_t *sym = lookup_table(n->token);
        if (!sym) { eval_error = UNDEFINED_SYMBOL; return 0; }
        return sym->val;
    }

    interior_node_t *in = (interior_node_t *)n->node;

    if (in->op == ASSIGN_OP) {
        if (in->left->type != LEAF || ((leaf_node_t *)in->left->node)->exp_type != SYMBOL) {
            eval_error = INVALID_LVALUE; return 0;
        }
        int val = eval_tree(in->right);
        if (eval_error == UNDEFINED_SYMBOL) { val = 0; eval_error = EVAL_NONE; }
        else if (eval_error != EVAL_NONE) return 0;

        symbol_t *s = lookup_table(in->left->token);
        if (!s) s = create_symbol(in->left->token, val);
        else s->val = val;
        if (!s) { eval_error = SYMTAB_FULL; return 0; }
        return val;
    }

    if (in->op == Q_OP) {
        int cond = eval_tree(in->left);
        if (eval_error) return 0;
        interior_node_t *alt = (interior_node_t *)in->right->node;
        return cond ? eval_tree(alt->left) : eval_tree(alt->right);
    }

    int l = eval_tree(in->left);
    if (eval_error) return 0;
    int r = eval_tree(in->right);
    if (eval_error) return 0;

    switch (in->op) {
        case ADD_OP: return l + r;
        case SUB_OP: return l - r;
        case MUL_OP: return l * r;
        case DIV_OP: if (!r) { eval_error = DIVISION_BY_ZERO; return 0; } return l / r;
        case MOD_OP: if (!r) { eval_error = INVALID_MODULUS; return 0; } return l % r;
        default: eval_error = UNKNOWN_OPERATION; return 0;
    }
}

void print_infix(tree_node_t *n)
{
    if (!n) return;
    if (n->type == LEAF) { printf("%s", n->token); return; }

    interior_node_t *in = (interior_node_t *)n->node;
    if (in->op == Q_OP) {
        printf("("); print_infix(in->left); printf("?");
        interior_node_t *alt = (interior_node_t *)in->right->node;
        printf("("); print_infix(alt->left); printf(":"); print_infix(alt->right); printf(")");
        printf(")");
    } else {
        printf("("); print_infix(in->left);
        printf("%s", in->op == ASSIGN_OP ? "=" : n->token);
        print_infix(in->right); printf(")");
    }
}

/* Your friend's rep() – unchanged */
void rep(char *exp)
{
    tree_node_t *tree = make_parse_tree(exp);
    if (tree == NULL) {
        return;
    }

    int val = eval_tree(tree);

    if (eval_error != EVAL_NONE) {
        cleanup_tree(tree);
        return;
    }

    print_infix(tree);
    printf(" = %d\n", val);

    cleanup_tree(tree);
}