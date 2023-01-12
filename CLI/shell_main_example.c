#include "callbacks.h"
#include "radix.h"
#include "misc.h"
#include "types.h"

int shell_main()
{
    radix_node *root_node;
    root_node = new_radix_node( "dot", NULL );

    if ( radix_insert(root_node, "show configuration ip") == NULL )
        printf("Error!\n");

    if ( radix_insert(root_node, "configure ip") == NULL )
        printf("Error!\n");

    if ( radix_lookup(root_node, "show configuration ip") == NULL )
        printf("Error!\n");

    if ( root_node->neighbor_arr[2] == NULL ) printf("\r\nFree slot found");

    return 0;
}
