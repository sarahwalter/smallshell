#include <stdlib.h>
#include <string.h>
#include "smsh_child_list.h"

/**
 * Pointer to the head node.
 */
struct smsh_child_node *head = NULL;

/**
 * Removes the specified node from the list.
 */
void remove(struct smsh_child_node* node);

/**
 * Adds child info to the front of the list.
 */
void add_child(struct smsh_child* child)
{
    if(head == NULL)
    {
        /** if head is null, there are no elements in the list 
         *  so we create the first one */
        head = (struct smsh_child_node*)malloc(sizeof(struct smsh_child_node));
        head->prev = NULL;
        head->next = NULL;
        memcpy(&head->child, child, sizeof(struct smsh_child));
    }
    else
    {
        /** create a new node for the data and append to the front */
        struct smsh_child_node* new_head = 
            (struct smsh_child_node*)malloc(sizeof(struct smsh_child_node));

        new_head->prev = NULL;
        new_head->next = head;
        memcpy(&new_head->child, child, sizeof(struct smsh_child));
        head = new_head;
    }
}

/**
 * Calls waitpid on each child in the list and returns a malloc'd pointer to 
 * the first one that is reaped. Note that more children may need to be reaped
 * after calling this function, so it is recommended to call this in a loop
 * until NULL is returned.
 *
 * Reaped child processes are also removed from the list.
 */
struct smsh_child* check_children()
{
    struct smsh_child* result = NULL;
    struct smsh_child_node* cur = head;
    while(cur != NULL && result == NULL)
    {
        int status;
        if(waitpid(head->child.pid, &status, WNOHANG) > 0)
        {
            /** child successfully reaped */
            result = (struct smsh_child*)malloc(sizeof(struct smsh_child));
            memcpy(result, &cur->child, sizeof(struct smsh_child));
            result->term_sig = WTERMSIG(status);
            result->exit_code = WEXITSTATUS(status);
            remove(cur);
        }
        else
        {
            cur = cur->next;
        }
    }

    return result;
}

/**
 * Removes a set of child into from the list.
 */
void remove(struct smsh_child_node* node)
{
    if(node == head)
    {
        /* remove from the front */
        head = head->next;
        if(head != NULL)
        {
            head->prev = NULL;
        }
    }
    else
    {
        /* remove from somewhere else in the list */
        struct smsh_child_node* cur_prev = node->prev;
        struct smsh_child_node* cur_next = node->next;

        if(cur_prev != NULL)
        {
            cur_prev->next = cur_next;
        }

        if(cur_next != NULL)
        {
            cur_next->prev = cur_prev;
        }
    }

    free(node);
}
