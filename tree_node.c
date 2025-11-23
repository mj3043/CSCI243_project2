// tree_node.c
// Parse tree node creation and cleanup
// @author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L   // must be first for strdup() with -pedantic

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree_node.h"

/// Create an interior node (operator)
/// @param op operator type
/// @param token operator string (e.g. "+", "?", ":")
/// @param left left subtree
/// @param right right subtree
/// @return new interior node or NULL on failure
tree_node_t *make_interior(op_type_t op, char *token,
                           tree_node_t *left, tree_node_t *right)
{
    tree_node_t *tn = malloc(sizeof(tree_node_t));
    if (!tn) {
        perror("malloc");
        return NULL;
    }

    tn->type = INTERIOR;
    tn->token = strdup(token ? token : "");
    if (!tn->token) {
        perror("strdup");
        free(tn);
        return NULL;
    }

    interior_node_t *in = malloc(sizeof(interior_node_t));
    if (!in) {
        perror("malloc interior_node_t");
        free(tn->token);
        free(tn);
        return NULL;
    }

    in->op    = op;
    in->left  = left;
    in->right = right;
    tn->node  = (void *)in;

    return tn;
}

/// Create a leaf node (integer or symbol)
/// @param exp_type INTEGER or SYMBOL
/// @param token string value
/// @return new leaf node or NULL on failure
tree_node_t *make_leaf(exp_type_t exp_type, char *token)
{
    tree_node_t *tn = malloc(sizeof(tree_node_t));
    if (!tn) {
        perror("malloc");
        return NULL;
    }

    tn->type = LEAF;
    tn->token = strdup(token ? token : "");
    if (!tn->token) {
        perror("strdup");
        free(tn);
        return NULL;
    }

    leaf_node_t *ln = malloc(sizeof(leaf_node_t));
    if (!ln) {
        perror("malloc leaf_node_t");
        free(tn->token);
        free(tn);
        return NULL;
    }

    ln->exp_type = exp_type;
    tn->node = (void *)ln;

    return tn;
}

/// Recursively free a parse tree
/// @param node root of tree/subtree to free (may be NULL)
void cleanup_tree(tree_node_t *node)
{
    if (!node) return;

    if (node->type == INTERIOR) {
        interior_node_t *in = (interior_node_t *)node->node;
        if (in) {
            cleanup_tree(in->left);
            cleanup_tree(in->right);
            free(in);
        }
    } else if (node->type == LEAF) {
        leaf_node_t *ln = (leaf_node_t *)node->node;
        if (ln) free(ln);
    }

    if (node->token) free(node->token);
    free(node);
}