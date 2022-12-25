#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "dynmatrix.h"

uint8_t** generate_pattern_match_matrix(uint8_t* pattern,
                                        uint16_t pattern_len,
                                        uint16_t payload_len,
                                        uint16_t* rows_count,
                                        uint16_t* columns_count)
{
    uint16_t matrix_rows = ( ( payload_len - pattern_len ) + 1 );
    uint8_t** matrix = (uint8_t **)malloc( sizeof(uint8_t *) * matrix_rows );

    uint8_t* normalized_pattern_buffer = (uint8_t *)malloc( sizeof(uint8_t) * payload_len );

    uint16_t i = 0;
    for ( i = 0; i < payload_len; i++ )
        *(normalized_pattern_buffer + i) = 'X';

    for ( i = 0; i < pattern_len; i++ )
        *(normalized_pattern_buffer + i) = pattern[i];


    printf("Inside function: single line from the configuration file: ");
    for ( i = 0; i < pattern_len; i++ )
    {    
        *(normalized_pattern_buffer + i) = pattern[i];
        printf( "%c", *(normalized_pattern_buffer + i) );
    }
    printf("\n");


    printf("Inside function: line transformed to packet buffer: ");
    for ( i = 0; i < payload_len; i++ )
        printf( "%c", *(normalized_pattern_buffer + i) );
    printf("\n");

    //
    // Populate the matrix
    //
    uint16_t offset = 0;
    for ( i = 0; i < matrix_rows; i++ )
    {
        matrix[i] = (uint8_t *)malloc( sizeof(uint8_t) * payload_len );
        for ( uint16_t j = 0; j < payload_len; j++ )
        {
            *( *(matrix + i) + ((j+offset) % payload_len) ) = normalized_pattern_buffer[j];
            
            // Alternative to: matrix[i][(j+offset) % payload_len] = normalized_pattern_buffer[j];
        }
        offset++;
    }

    *rows_count = matrix_rows;
    *columns_count = payload_len;

    free(normalized_pattern_buffer);

    return matrix;
}

void print_pattern_match_matrix(uint8_t** matrix,
                                uint16_t rows_count,
                                uint16_t columns_count)
{
    uint16_t i = 0;

    printf("===========================\n");
    for ( i = 0; i < rows_count; i++ )
    {
        printf("Row %u: ", i);
        
        for ( uint16_t j = 0; j < columns_count; j++ )
        {    
            printf( "%c", *( *(matrix + i) + j) ); 
            
            // Alternative to: printf("%c", matrix[i][j]);
        }
        printf("\n");
    }

    return;
}

int main(void) 
{
    //
    // This is the packet buffer
    //
    uint16_t BUFF_LEN = 10; // The size should be a configuration option (related to the MTU, MSS, etc)
    // uint8_t packet[BUFF_LEN] = { 'h', 'e', 'l', 'l', 'o', 'a', '5', 'b', 'j', '8' }; // This is the network packet - the only static array in the example

    //
    // This a parsed line from the configuration file with the signatures
    //
    uint16_t PATTERN_LEN = 3; // Imagine we have parsed a line that is 3-bytes in length
    uint8_t* pattern = (uint8_t *)malloc( sizeof(uint8_t) * PATTERN_LEN );
    pattern[0] = 'x';
    pattern[1] = 'y';
    pattern[2] = 'z';

    uint16_t rows = 0, columns = 0;
    uint8_t** matrix = generate_pattern_match_matrix(pattern, PATTERN_LEN, BUFF_LEN, &rows, &columns);
    print_pattern_match_matrix(matrix, rows, columns);

    free( matrix );

    printf("===========================\n");

    return 0;
}