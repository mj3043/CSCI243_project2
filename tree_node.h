// @author: tvf

#ifndef TREE_NODE_H
#define TREE_NODE_H

#include "symtab.h"

// Operation tokens
#define ADD_OP_STR	"+"
#define SUB_OP_STR	"-"
#define MUL_OP_STR	"*"
#define DIV_OP_STR	"/"
#define MOD_OP_STR	"%"
#define Q_OP_STR        "?"
#define ASSIGN_OP_STR	"="

// valud operations types for interior nodes
typedef enum op_type_e {
    ADD_OP,                     // addition
    SUB_OP,                     // subtraction
    MUL_OP,                     // multiplication
    DIV_OP,                     // division
    MOD_OP,                     // modulus
    ASSIGN_OP,                  // assignment
    Q_OP,                       // ternary operation
    ALT_OP,                     // alternatives (for ternary operation)
    NO_OP                       // not an op
} op_type_t;

// Valid types of leaf nodes (tells how to interpret the token)
typedef enum exp_type_e {
    INTEGER,                    // constant integer literal
    SYMBOL                      // a variable name
} exp_type_t;

// Valid tree_node types
typedef enum node_type_e {
    INTERIOR,
    LEAF
} node_type_t;

// Represents a node in the parse tree
typedef struct tree_node_s {
    node_type_t type;           // the type of the node
    char *token;                // the token that derived this node
    void *node;                 // either an interiorNode or leafNode
} tree_node_t;

typedef struct interior_node_s {
    op_type_t op;                  // the operation at this interior node
    tree_node_t *left;          // the left operand
    tree_node_t *right;         // the right operand
} interior_node_t;

typedef struct leaf_node_s {
    exp_type_t exp_type;        // INTEGER or SYMBOL
} leaf_node_t;

// Construct an interior node dynamically on the heap.
// @param op  the operation (add, subtract, etc.)
// @param token  the token that derives this node
// @param left  pointer to the left child of this node
// @param right pointer to the right child of this node
// @return the new TreeNode, or NULL if error
tree_node_t *make_interior(op_type_t op, char *token,
                       tree_node_t *left, tree_node_t *right);

// Construct a leaf node dynamically on the heap.
// @param expType  the operation token type (INTEGER or SYMBOL)
// @param token  the token that derives this node
// @return the new TreeNode, or NULL if error
tree_node_t *make_leaf(exp_type_t exp_type, char *token);

#endif
