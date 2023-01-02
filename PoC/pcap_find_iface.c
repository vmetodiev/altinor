//
// Compile with: 
// $ gcc pcap_find_iface.c -o pcap_find_iface -lpcap
//
#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>

int main(void)
{
    char *device;
    char error_buffer[PCAP_ERRBUF_SIZE];

    device = pcap_lookupdev(error_buffer);
    
    if (device == NULL) {
        printf("Error finding device: %s\n", error_buffer);
        return 1;
    }

    printf("Network device found: %s\n", device);
    
    return 0;
}
