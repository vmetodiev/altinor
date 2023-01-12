#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* UINT becoming cool */
#define  BYTE int8_t
#define  WORD int16_t
#define  DWORD int32_t

typedef enum { 0, 1 } BOOL;

/* String */
#define STRING_LEN 255
typedef unsigned char String[STRING_LEN];

/* Function pointers - naming by parameters used */
// any callback typedef ...
typedef void (*FunctionPtrType_Int)       (int);
typedef int  (*FunctionPtrType_IntChar)   (int, char);
/* and so on */


#endif // TYPES_H_INCLUDED
