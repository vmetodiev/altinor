#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

int main(void) 
{
    //
    // This is the packet buffer
    //
    int BUFF_LEN = 10; // The size should be a configuration option (related to the MTU, MSS, etc)
    uint8_t packet[BUFF_LEN] = { 'h', 'e', 'l', 'l', 'o', 'a', '5', 'b', 'j', '8' }; // This is the network packet - the only static array in the example

    //
    // This a parsed line from the configuration file with the signatures
    //
    int PATTERN_LEN = 3; // Imagine we have parsed a line that is 3-bytes in length
    uint8_t* pattern = (uint8_t *)malloc( sizeof(uint8_t) * PATTERN_LEN );
    pattern[0] = 'x';
    pattern[1] = 'y';
    pattern[2] = 'z';

    int i = 0;
    uint8_t* pattern_buffer = (uint8_t *)malloc( sizeof(uint8_t) * BUFF_LEN );

    for ( i = 0; i < BUFF_LEN; i++ )
        *(pattern_buffer + i) = 'X';

    printf("Single line from the configuration file: ");
    for ( i = 0; i < PATTERN_LEN; i++ )
    {    
        *(pattern_buffer + i) = pattern[i];
        printf( "%c", *(pattern_buffer + i) );
    }
    printf("\n");

    int matrix_rows = ( ( BUFF_LEN - PATTERN_LEN ) + 1 );
    uint8_t** rows = (uint8_t **)malloc( sizeof(uint8_t *) * matrix_rows );

    printf("Line transformed to packet buffer: ");
    for ( i = 0; i < BUFF_LEN; i++ )
        printf( "%c", *(pattern_buffer + i) );
    printf("\n");

    //
    // Populate the matrix
    //
    int offset = 0;
    for ( i = 0; i < matrix_rows; i++ )
    {
        rows[i] = (uint8_t *)malloc( sizeof(uint8_t) * BUFF_LEN );
        for ( int j = 0; j < BUFF_LEN; j++ )
        {
            *( *(rows + i) + ((j+offset) % BUFF_LEN) ) = pattern_buffer[j];
            
            // Alternative to: 
            // rows[i][(j+offset) % BUFF_LEN] = pattern_buffer[j];
        }
        offset++;
    }

    //
    // Print the matrix
    //
    printf("===========================\n");
    for ( i = 0; i < matrix_rows; i++ )
    {
        printf("Row %d: ", i);
        
        for ( int j = 0; j < BUFF_LEN; j++ )
        {    
            printf( "%c", *( *(rows + i) + j) ); 
            
            // Alternative to: 
            // printf("%c", rows[i][j]);
        }
        printf("\n");
    }

    free( rows );
    free( pattern_buffer );

    printf("===========================\n");

    return 0;
}