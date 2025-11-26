// stack.c
// Generic LIFO stack for holding owned string tokens
// Memory-safe: every pushed string is duplicated with strdup()
// @author: Munkh-Orgil Jargalsaikhan

#define _POSIX_C_SOURCE 200809L   // must be first for strdup() with -pedantic

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"                 // only include once!

/// Create and initialize a new empty stack
/// @return pointer to new stack, exits on allocation failure
stack_t *make_stack(void)
{
    stack_t *s = malloc(sizeof(stack_t));
    if (!s) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    s->top = NULL;
    return s;
}

/// Push a copy of data onto the stack
/// If data is non-NULL, it is duplicated with strdup() - stack owns the copy
/// @param stack target stack
/// @param data pointer to data (expected to be a string)
void push(stack_t *stack, void *data)
{
    if (!stack) return;

    stack_node_t *node = malloc(sizeof(stack_node_t));
    if (!node) {
        perror("malloc node");
        exit(EXIT_FAILURE);
    }

    if (data) {
        node->data = strdup((char *)data);
        if (!node->data) {
            perror("strdup");
            free(node);
            exit(EXIT_FAILURE);
        }
    } else {
        node->data = NULL;
    }

    node->next = stack->top;
    stack->top = node;
}

/// Return pointer to data at top of stack (does not remove it)
/// @param stack the stack
/// @return pointer to top data, exits on empty stack
void *top(stack_t *stack)
{
    if (!stack || !stack->top) {
        fprintf(stderr, "attempt to retrieve the top of an empty stack\n");
        exit(EXIT_FAILURE);
    }
    return stack->top->data;
}

/// Remove and free the top element of the stack
/// Also frees the duplicated string stored in data
/// @param stack the stack
void pop(stack_t *stack)
{
    if (!stack || !stack->top) {
        fprintf(stderr, "attempt to pop from an empty stack\n");
        exit(EXIT_FAILURE);
    }

    stack_node_t *node = stack->top;
    stack->top = node->next;

    if (node->data) free(node->data);   // free owned token
    free(node);
}

/// Check if stack is empty
/// @param stack the stack
/// @return 1 if empty (or stack is NULL), 0 otherwise
int empty_stack(stack_t *stack)
{
    if (!stack) return 1;
    return (stack->top == NULL) ? 1 : 0;
}

/// Free entire stack and all owned data
/// @param stack stack to free (also frees the stack_t itself)
void free_stack(stack_t *stack)
{
    if (!stack) return;

    while (stack->top) {
        stack_node_t *node = stack->top;
        stack->top = node->next;
        if (node->data) free(node->data);
        free(node);
    }
    free(stack);
}