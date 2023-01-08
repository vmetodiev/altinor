#include <iostream>
#include <stdint.h>

int main()
{
    uint32_t payload_len = 15;
    uint8_t payload_ptr[payload_len] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    
    const uint32_t CL_PAYLOAD_CHUNK_LEN = 4; 
    
    uint32_t loop_payload_len = payload_len;
    uint32_t offset = 0;    
 
    if ( (payload_len / CL_PAYLOAD_CHUNK_LEN) > 0 )
    {
        do {
    
            for ( int i = 0; i < CL_PAYLOAD_CHUNK_LEN; i++ )
            {
                //
                // Do the OpenCL things
                //
                std::cout << payload_ptr[offset + i] << std::endl;
            }
    
            offset += CL_PAYLOAD_CHUNK_LEN;
            loop_payload_len -= CL_PAYLOAD_CHUNK_LEN;
                
            std::cout << "****************" << std::endl;
            
        } while ( loop_payload_len >= CL_PAYLOAD_CHUNK_LEN );
        
        if ( loop_payload_len > 0 )
        {
            // Set the offset to point to the last chuck
            offset = payload_len - CL_PAYLOAD_CHUNK_LEN;
            
            for ( int i = offset; i < payload_len; i++ )
            {
                //
                // Do the OpenCL things
                //
                std::cout << payload_ptr[i] << std::endl;
            }
        }
    
    }
    
    return 0;
}