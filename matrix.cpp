#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define BUFF_LEN 10

//
// Imagine that we have parsed a 3-byte signature by the configuration file
//
#define PATTERN_LEN 3
uint8_t pattern[PATTERN_LEN] = { 'a', '5', 'b' }; 
//
//
//

int main(int argc, char ** argv) {
    
    int i = 0;
	uint8_t packet[BUFF_LEN] =  { 'h', 'e', 'l', 'l', 'o', 'a', '5', 'b', 'j', '8' };    
    uint8_t pattern_buffer[BUFF_LEN];
    
    for ( i = 0; i < BUFF_LEN; i++ )
        pattern_buffer[i] = 'X';

    printf("Parsed pattern: ");
    for ( i = 0; i < PATTERN_LEN; i++ )
    {    
        pattern_buffer[i] = pattern[i];
        printf("%c", pattern_buffer[i]);
    }
    printf("\n");

    int matrix_rows = ( ( BUFF_LEN - PATTERN_LEN ) + 1 );
    uint8_t matrix[matrix_rows][BUFF_LEN] = { 0x00 };

    printf("Buffer: ");
    for ( i = 0; i < BUFF_LEN; i++ )
        printf("%c", pattern_buffer[i]);
    printf("\n");

    //
    // Populate the matrix
    //
    int offset = 0;
    for ( i = 0; i < matrix_rows; i++ )
    {
        for ( int j = 0; j < BUFF_LEN; j++ )
        {
            matrix[i][(j+offset) % BUFF_LEN] = pattern_buffer[j];
        }
        offset++;
    }

    //
    // Print the matrix
    //
    for ( i = 0; i < matrix_rows; i++ )
    {
        printf("Row %d: ", i);
        
        for ( int j = 0; j < BUFF_LEN; j++ )
            printf("%c", matrix[i][j]);
        
        printf("\n");
    }

    return 0;
}