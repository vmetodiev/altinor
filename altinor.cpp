//
// Prerequisites:
// sudo apt install opencl-headers
// sudo apt install ocl-icd-opencl-dev
//

#include <stdio.h>
#include <CL/cl.h>
#include <string.h>
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

#include "dynmatrix.h"
#include "altinor.h"

const uint32_t PATTERN_LEN = SIGNATURE_LEN;
const uint32_t CL_PAYLOAD_CHUNK_LEN = 256;

uint16_t rows = 0;
uint16_t cols = 0;

volatile uint32_t c = 0;

size_t globalItemSize = CL_PAYLOAD_CHUNK_LEN;

cl_uint clDimensions  = 1;
cl_command_queue commandQueue = NULL;
cl_kernel kernel = NULL;

cl_mem aMemObj = NULL;
cl_mem bMemObj = NULL;
cl_mem cMemObj = NULL;
cl_mem dMemObj = NULL;
cl_mem eMemObj = NULL;
cl_mem fMemObj = NULL;

void handle_packet(unsigned char* buffer, int buf_len)
{
	struct iphdr *ip = (struct iphdr*)( buffer + sizeof(struct ethhdr) );
	uint16_t iphdrlen = ip->ihl*4;
	
	uint8_t *payload_ptr = NULL;
	uint32_t payload_len =  0;

	#if ( INSPECT_UDP == 1 )
	if ( ip->protocol == UDP_PACKET )
	{
		payload_ptr = ( buffer + sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr) );
		payload_len =  ( buf_len - ( sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr) ) );

		#if ( ENABLE_LOG == 1 )
			printf("UDP Payload from IP %.4X \n", ip->saddr);
			printf("UDP %u bytes payload from IP %s \n", payload_len, inet_ntoa( *(struct in_addr *)&ip->saddr) );
			for ( uint32_t i = 0; i < payload_len; i++ )
				printf("%c", payload_ptr[i]); // %.2X
			
			printf( "_______________________________\n" );
		#endif

		goto inspect_pkt;
	}
	#endif

	#if ( INSPECT_TCP == 1 )
	if ( ip->protocol == TCP_PACKET )
	{		
		struct tcphdr *tcp = (struct tcphdr *)((char *)ip + iphdrlen);
		uint16_t tcphdrlen = (tcp->doff * 4);

		payload_ptr = ( buffer + sizeof(struct ethhdr) + iphdrlen + tcphdrlen );		
		payload_len = ntohs(ip->tot_len) - tcphdrlen - iphdrlen;

		#if ( ENABLE_LOG == 1 )
			printf("TCP Payload from IP %.4X \n", ip->saddr);
			printf("TCP %u bytes payload from IP %s \n", payload_len, inet_ntoa( *(struct in_addr *)&ip->saddr) );
			for ( uint32_t i = 0; i < payload_len; i++ )
				printf("%c", payload_ptr[i]); // %.2X
			
			printf( "_______________________________\n" );
		#endif

		goto inspect_pkt;
	}
	#endif

	inspect_pkt:
		cl_int ret;
		ELEMENT_TYPE* a = payload_ptr;

		uint32_t loop_payload_len = payload_len;
    	uint32_t offset = 0;
 
		if ( (payload_len / CL_PAYLOAD_CHUNK_LEN) > 0 )
		{
			do {
				#if ( ENABLE_LOG == 1 )
					for ( uint32_t i = offset; i < offset + CL_PAYLOAD_CHUNK_LEN; i++ )
						printf("%c", payload_ptr[i]);
				#endif

				OPENCL_MATCH_LOGIC();
				
				offset += CL_PAYLOAD_CHUNK_LEN;
				loop_payload_len -= CL_PAYLOAD_CHUNK_LEN;

			} while ( loop_payload_len >= CL_PAYLOAD_CHUNK_LEN );
			
			#if ( ENABLE_LOG == 1 )
				printf("loop_payload_len: %u\n", loop_payload_len);
				printf("payload_len: %u\n", payload_len);
			#endif

			if ( loop_payload_len > 0 )
			{
				// Set the offset to point to the last chuck
				offset = payload_len - CL_PAYLOAD_CHUNK_LEN;
				
				#if ( ENABLE_LOG == 1 )
					printf("end offset: %u\n", offset);
					
					for ( uint32_t i = offset; i < offset + CL_PAYLOAD_CHUNK_LEN; i++ )
						printf("%c", payload_ptr[i]);
				#endif

				OPENCL_MATCH_LOGIC();
			}
		}

		else if ( payload_len > 0 )
		{
			offset = 0;	

			#if ( ENABLE_LOG == 1 )
			for ( uint32_t i = 0; i < payload_len; i++ )
					printf("%c", payload_ptr[i+offset]);
			#endif
				
			OPENCL_MATCH_LOGIC();
		}
	
	return;
}

int main(int argc, char ** argv) 
{	
	if ( PATTERN_LEN >= CL_PAYLOAD_CHUNK_LEN )
	{
		printf("Fatal: PATTERN_LEN is greater than or equal to the CL_PAYLOAD_CHUNK_LEN!\n");
	}

	ELEMENT_TYPE* pattern = ( ELEMENT_TYPE* )malloc( sizeof(ELEMENT_TYPE) * CL_PAYLOAD_CHUNK_LEN );
	for ( uint32_t i = 0; i < CL_PAYLOAD_CHUNK_LEN; i++ )
		pattern[i] = 0x00;
	
	for ( uint32_t i = 0; i < SIGNATURE_LEN; i++ )
		pattern[i] = signature[i];

    ELEMENT_TYPE** matrix = generate_pattern_match_matrix( pattern, PATTERN_LEN, CL_PAYLOAD_CHUNK_LEN, &rows, &cols );
    // print_pattern_match_matrix(matrix, rows, cols);

    ELEMENT_TYPE* buffer = generate_pattern_match_array( matrix, rows, cols );

	printf("Generated buffer len: %u\n", (rows * cols) );

	ELEMENT_TYPE* b = buffer;
	uint32_t d = PATTERN_LEN;    // Singature length
	uint32_t e = rows;           // Parts  (rows in the pattern tables foreach signature)
	uint32_t f = cols;

	if ( cols != CL_PAYLOAD_CHUNK_LEN )
	{
		printf( "Pattern Matrix column count (%u) does not match CL_PAYLOAD_CHUNK_LEN!\n", cols );
		return -1;
	}

	// Load kernel from file vecAltinorKernel
	FILE *kernelFile;
	char *kernelSource;
	size_t kernelSize;

	kernelFile = fopen("vecAltinorKernel.cl", "r");

	if (!kernelFile) {

		fprintf(stderr, "No file named vecAltinorKernel.cl was found\n");
		exit(-1);
	}

	kernelSource = (char*)malloc(MAX_SOURCE_SIZE);
	kernelSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
	fclose(kernelFile);

	// Getting platform and device information
	cl_platform_id platformId = NULL;
	cl_device_id deviceID = NULL;
	cl_uint retNumDevices;
	cl_uint retNumPlatforms;
	cl_int ret = clGetPlatformIDs(1, &platformId, &retNumPlatforms);
	ret = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_DEFAULT, 1, &deviceID, &retNumDevices);

	// Check the OpenCL version
	char cBuffer[1024];
	clGetDeviceInfo(deviceID, CL_DEVICE_OPENCL_C_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	printf("%s\n", cBuffer);

	// Creating context.
	cl_context context = clCreateContext(NULL, 1, &deviceID, NULL, NULL, &ret);

	// Creating command queue
	commandQueue = clCreateCommandQueueWithProperties(context, deviceID, 0, &ret);

	// Memory buffers for each array - CL_MEM_ALLOC_HOST_PTR
	aMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, CL_PAYLOAD_CHUNK_LEN * sizeof(ELEMENT_TYPE), NULL, &ret);
	bMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, rows * cols * sizeof(ELEMENT_TYPE), NULL, &ret);
	dMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &ret); 
	eMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &ret); 
	fMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &ret);

	cMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), NULL, &ret); 

	// Create program from kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, (const size_t *)&kernelSize, &ret);

	// Build program
	ret = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);

	// Create kernel
	kernel = clCreateKernel(program, "matchVectors", &ret);

	// Set arguments for kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&aMemObj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bMemObj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cMemObj);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&dMemObj);
	ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&eMemObj);
	ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&fMemObj);

	// Buffer 'a' is enqued inside handle_packet
	ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, rows * cols * sizeof(ELEMENT_TYPE), b, 0, NULL, NULL);
	// Buffer 'c' is enqued inside handle_packet
	ret = clEnqueueWriteBuffer(commandQueue, dMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&d), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, eMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&e), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, fMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&f), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&c), 0, NULL, NULL);

	int sock_raw_fd, saddr_len, buf_len;
	struct sockaddr saddr;

	unsigned char* pkt_buffer = (unsigned char *)malloc(RCV_BUFFER_SIZE); 
	memset(pkt_buffer, 0, RCV_BUFFER_SIZE);

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

	//
	// Main loop
	// 
	while ( 1 )
	{
		saddr_len = sizeof(saddr);
		buf_len = recvfrom(sock_raw_fd, pkt_buffer, RCV_BUFFER_SIZE, 0, &saddr, (socklen_t *)&saddr_len);

		if( buf_len < 0 )
		{
			printf("ERROR: recvfrom()\n");
			return -1;
		}

		handle_packet(pkt_buffer, buf_len);
	}
	//
	// End of Main loop
	//

	close(sock_raw_fd);

	ret = clFlush(commandQueue);
	ret = clFinish(commandQueue);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);

	ret = clReleaseMemObject(aMemObj);
	ret = clReleaseMemObject(bMemObj);
	ret = clReleaseMemObject(cMemObj);
	ret = clReleaseMemObject(dMemObj);
	ret = clReleaseMemObject(eMemObj);
	ret = clReleaseMemObject(fMemObj);
	
	ret = clReleaseContext(context);

	free(pattern);
	free(buffer);

	printf("Terminating...\n");
	return 0;
}