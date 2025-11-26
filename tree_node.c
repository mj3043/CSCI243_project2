// tree_node.c
// Fixed – frees token strings correctly
// Author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree_node.h"

tree_node_t *make_interior(op_type_t op, char *token,
                           tree_node_t *left, tree_node_t *right)
{
    tree_node_t *tn = malloc(sizeof(tree_node_t));
    if (!tn) return NULL;
    tn->type = INTERIOR;
    tn->token = strdup(token ? token : "");
    if (!tn->token) { free(tn); return NULL; }

    interior_node_t *in = malloc(sizeof(interior_node_t));
    if (!in) { free(tn->token); free(tn); return NULL; }

    in->op = op; in->left = left; in->right = right;
    tn->node = (void *)in;
    return tn;
}

tree_node_t *make_leaf(exp_type_t exp_type, char *token)
{
    tree_node_t *tn = malloc(sizeof(tree_node_t));
    if (!tn) return NULL;
    tn->type = LEAF;
    tn->token = strdup(token ? token : "");
    if (!tn->token) { free(tn); return NULL; }

    leaf_node_t *ln = malloc(sizeof(leaf_node_t));
    if (!ln) { free(tn->token); free(tn); return NULL; }

    ln->exp_type = exp_type;
    tn->node = (void *)ln;
    return tn;
}

void cleanup_tree(tree_node_t *node)
{
    if (!node) return;

    if (node->type == INTERIOR) {
        interior_node_t *in = (interior_node_t *)node->node;
        cleanup_tree(in->left);
        cleanup_tree(in->right);
        free(in);
    } else if (node->type == LEAF) {
        free(node->node);
    }

    free(node->token);   // Fixed – was missing before
    free(node);
}