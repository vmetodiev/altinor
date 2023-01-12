#include "../CLI/radix.h"
#include "common.h"
#include <string.h>
#include "../CLI/misc.h"

/* Initialize the radix tree */
radix_node* new_radix_node ( String cmd, void (*callback_action)() )
{
    radix_node* root = (radix_node *)malloc( sizeof(radix_node) );

    if ( root == NULL )
        return FALSE;

    WORD i = 0;
    for ( i = 0; i < STRING_LEN; i++ )
        root->command[i] = cmd[i];

    root->function_ptr = callback_action;

    for ( i = 0; i < MAX_NEIGH; i++ )
        root->neighbor_arr[i] = NULL;

    return root;
}

/* Radix lookup function*/
radix_node* radix_lookup (radix_node *root, String str)
{
    radix_node *current = root;
    String temp = "";

    /* split the string, lookup word by word */
    BYTE words = 0, i = 0;
    words = str_count_words(str);

    // head reads until space, lookup the word in between
    WORD head = 0, tail = 0;

    for (i = 0; i < words; i++) {
        // move the head until reaching ' ' character
        while( str[head] != ' ')
            head++;

        // extract string
        memset( temp, 0, strlen( (const char *)temp) );     // clear buffer
        strncpy( (char *)temp, (const char *)str + tail, (size_t)head - tail );
        //printf( "\ntemp=.%s.", temp );

        head++;                                        // jump to the next word
        tail = head;                                   // update tail

        // compare in neighbor array
        current = goto_child_node(current, temp);
        if ( current == NULL )
            break;

    }

    return current;
}

/* Radix insert node function */
radix_node* radix_insert (radix_node *root, String str)
{
    radix_node *new_node = NULL;
    String temp = "";

    radix_node *insert_point = root;
    radix_node *current = root;
    /* split the string, lookup word by word */
    BYTE words = 0, i = 0;
    words = str_count_words(str);
    printf("\r\n%u words present", words);
    // head reads until space, lookup the word in between
    WORD head = 0, tail = 0;

    for (i = 0; i < words; i++) {
        // move the head until reaching ' ' character
        while( str[head] != ' ')
            head++;

        // extract string
        memset( temp, 0, strlen( (const char *)temp) );               // clear buffer
        strncpy( (char *)temp, (const char *)str + tail, (size_t)head - tail );
        printf( "\ntemp=.%s.", temp );

        head++;                                        				// jump to the next word
        tail = head;                                   				// update tail

        // compare in neighbor array
        current = goto_child_node( current, temp );
        if ( current == NULL ){
            // insert point reached, check for a free slot to insert the new node
            new_node = new_radix_node( temp, NULL );
            WORD free_slot_id = find_free_slot_in_neigh_array(insert_point);
            insert_point->neighbor_arr[free_slot_id] = new_node;
            insert_point = current = new_node;
        }
        else {
            insert_point = current;
        }
    }

    return current;
}

/* Radix delete node function */
BOOL radix_delete (radix_node *root, String str)
{
    return TRUE;
}

// Helper functions here
radix_node* goto_child_node(radix_node *current, String str)
{
    BYTE i = 0;
    for ( i = 0; i < MAX_NEIGH; i++ )
    {
        if ( current->neighbor_arr[i] == NULL ) {
            return NULL;
        }
        else {
            if ( strncmp( (const char *)str, (const char *)current->neighbor_arr[i]->command, (size_t)strlen((const char *)str)) == 0 )
            {
                //return child
                return current->neighbor_arr[i];
            }
        }
    }

    return NULL; // null pointer if no such child
}

BYTE find_free_slot_in_neigh_array(radix_node *current)
{
    BYTE i = 0;
    for ( i = 0; i < MAX_NEIGH; i++ )
    {
        if ( current->neighbor_arr[i] == NULL )
            return i;
    }

    // Count to infinite, invalid value if no free slot found
    return MAX_NEIGH;
}
