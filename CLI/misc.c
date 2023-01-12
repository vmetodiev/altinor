#include "../CLI/misc.h"

BYTE str_count_words(String str)
{
    BYTE cnt = 0, ptr = 0;
    while ( str[ptr] != '\0' )
    {
        if ( !isalpha(str[ptr]) )
        {
            if ( ( str[ptr] == ' ' ) &&  isalpha( str[ptr - 1] ) && isalpha( str[ptr + 1] ) )
                cnt++;
            else
                return 0; // neither alpha nor space character - invalid non-terminal
        }
        ptr++;
    }

    return ++cnt;

}
