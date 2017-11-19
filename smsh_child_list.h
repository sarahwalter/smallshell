#ifndef SMSH_CHILD_LIST_H
#define SMSH_CHILD_LIST_H

#include "smsh_types.h"

/**
 * Node structure for basic linked list.
 */
struct smsh_child_node
{
    struct smsh_child_node *prev;
    struct smsh_child_node *next;
    struct smsh_child child;
};

/**
 * checks the list of child processes and returns a malloc'd pointer
 * to a child structure that was reaped. If no child was reaped then
 * this function returns NULL.
 *
 * Clients can ccontinue to call this method until NULL is returned
 * to get information for all children that have terminated.
 *
 * Clients must free the memory of the returned pointer to avoid leaks.
 */
struct smsh_child* check_children();

/**
 * Adds a child structure to the list. This module does not assume
 * ownership of the parameter.
 */
void add_child(struct smsh_child* child);

#endif
