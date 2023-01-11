//
// Run as root
// 

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <netinet/in.h>		 
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define ETH_IFACE_NAME "enp2s0"
#define RCV_BUFFER_SIZE 65536

#define TCP_PACKET  6
#define UDP_PACKET 17

void handle_packet(unsigned char* buffer, int buf_len)
{
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof (struct ethhdr));
	uint16_t iphdrlen = ip->ihl*4;
	
	uint8_t *data = NULL;
	uint32_t payload_len =  0;

	if ( ip->protocol == UDP_PACKET )
	{
		data = ( buffer + sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr) );
		payload_len =  ( buf_len - ( sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr) ) );

		//printf("UDP Payload from IP %.4X \n", ip->saddr);
		printf("UDP %u bytes payload from IP %s \n", payload_len, inet_ntoa( *(struct in_addr *)&ip->saddr) );
		for ( int i = 0; i < payload_len; i++ )
			printf("%c", data[i]); // %.2X
		
		printf( "_______________________________\n" );
	}

	if ( ip->protocol == TCP_PACKET )
	{		
		struct tcphdr *tcp = (struct tcphdr *)((char *)ip + iphdrlen);
		uint16_t tcphdrlen = (tcp->doff * 4);

		data = ( buffer + sizeof(struct ethhdr) + iphdrlen + tcphdrlen );		
		payload_len = ntohs(ip->tot_len) - tcphdrlen - iphdrlen;

		//printf("TCP Payload from IP %.4X \n", ip->saddr);
		printf("TCP %u bytes payload from IP %s \n", payload_len, inet_ntoa( *(struct in_addr *)&ip->saddr) );
		for ( int i = 0; i < payload_len; i++ )
			printf("%c", data[i]); // %.2X
		
		printf( "_______________________________\n" );
	}
}

int main()
{
	int sock_raw_fd, saddr_len, buf_len;
	struct sockaddr saddr;

	unsigned char* buffer = (unsigned char *)malloc(RCV_BUFFER_SIZE); 
	memset(buffer, 0, RCV_BUFFER_SIZE);

	printf("INFO: Starting... \n");

	sock_raw_fd = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
	if( sock_raw_fd < 0 )
	{
		printf("FATAL: Error creating a raw socket!\n");
		return -1;
	}
	
	int success = setsockopt( sock_raw_fd, SOL_SOCKET, SO_BINDTODEVICE, ETH_IFACE_NAME, strlen(ETH_IFACE_NAME) );
	if ( success != 0 )
		printf("ERROR: Could not bind to iface %s with ret val: %d\n", ETH_IFACE_NAME, success);

	while ( 1 )
	{
		saddr_len = sizeof(saddr);
		buf_len = recvfrom(sock_raw_fd, buffer, RCV_BUFFER_SIZE, 0, &saddr, (socklen_t *)&saddr_len);

		if( buf_len < 0 )
		{
			printf("ERROR: recvfrom()\n");
			return -1;
		}
		handle_packet(buffer, buf_len);
	}

	close(sock_raw_fd);
	printf("Terminating...\n");

}
