#ifndef RADIX_H_INCLUDED
#define RADIX_H_INCLUDED

#include "callbacks.h"
#include "types.h"

#define MAX_NEIGH 5
/* Radix node definition */
typedef struct radix_node
{
    String command;
    void (*function_ptr)();
    struct radix_node *neighbor_arr[MAX_NEIGH];
} radix_node;

/* Radix tree methods */
typedef struct radix_operations
{
    // All function pointers to the operations */
} radix_operations;

/* Initialize the radix tree */
radix_node* new_radix_node ( String cmd, void (*callback_action)() );

/* Radix lookup function*/
radix_node* radix_lookup (radix_node *root, String str);

/* Radix insert node function */
radix_node* radix_insert (radix_node *root, String str);

/* Radix delete node function */
BOOL radix_delete (radix_node *root, String str);

// Should be overridden by the CLI.h functionality
//extern radix_node root_nd;

// Helper functions here
radix_node* goto_child_node(radix_node *current, String str);
BYTE find_free_slot_in_neigh_array(radix_node *current);

#endif // RADIX_H_INCLUDED
