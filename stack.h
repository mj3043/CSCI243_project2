// @author: tvf

#ifndef STACK_H
#define STACK_H

#include "stack_node.h"

typedef struct stack_s {
    stack_node_t *top;             // linked list of stack_nodes
} stack_t;

/// make a new stack
/// @return  a new stack structure with a NULL top pointer
stack_t *make_stack(void);

/// Add an element to the top of the stack (stack is changed).
/// This routine should dynamically allocate a new node.
/// @param stack Points to the stack 
/// @param data The token (C String)
void push(stack_t *stack, void *data);

/// Return the top element from the stack (stack is unchanged)
/// @param stack points to the stack
/// @return the top element on the stack (a pointer to something)
/// @exception If the stack is empty, the program should 
///     exit with EXIT_FAILURE
void *top(stack_t * stack);

/// Removes the top element from the stack (stack is changed).
/// This routine should free the memory associated with the top node.
/// @param stack points to the stack
/// @exception If the stack is empty, the program should 
///     exit with EXIT_FAILURE
void pop(stack_t *stack);

/// Tells whether the stack is empty or not (stack is unchanged)
/// @param stack Points to the stack
/// @return 0 if not empty, any other value otherwise
int empty_stack(stack_t * stack);

/// Frees all of the stack nodes, including the stack structure
/// @param stk  Points to the stack to free
void free_stack(stack_t * stack);

#endif
