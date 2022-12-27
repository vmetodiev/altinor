//
// Prerequisites:
// sudo apt install opencl-headers
// sudo apt install ocl-icd-opencl-dev
//
// Compile with:
// c++ experimentNG.cpp -o experimentNG -l OpenCL 
// (or g++)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dynmatrix.h"
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)
#define ELEMENT_TYPE uint8_t

int main(int argc, char ** argv) 
{	
    const uint32_t PATTERN_LEN = 5;
    const uint32_t PAYLOAD_LEN = 255;

	if ( PATTERN_LEN >= PAYLOAD_LEN )
	{
		printf("Fatal: PATTERN_LEN is greater than or equal to the PAYLOAD_LEN!\n");
	}

	ELEMENT_TYPE* pattern = ( ELEMENT_TYPE* )malloc( sizeof(ELEMENT_TYPE) * PAYLOAD_LEN );
	for ( uint32_t i = 0; i < PAYLOAD_LEN; i++ )
		pattern[i] = 'M';
	
	// Manually define the pattern
	pattern[0] = 'A';
	pattern[1] = 'M';
	pattern[2] = 'D';
	pattern[3] = '*';
	pattern[4] = 'a';
	pattern[5] = 'm';
	pattern[6] = 'd';

	// The packet the may contain the pattern
	ELEMENT_TYPE* payload = ( ELEMENT_TYPE* )malloc( sizeof(ELEMENT_TYPE) * PAYLOAD_LEN );
	for ( uint32_t i = 0; i < PAYLOAD_LEN; i++ )
		payload[i] = 'M';
	
	// Manually insert some bytes inside the packet (payload)
	int offset = 0;
	payload[PAYLOAD_LEN - 7 - offset] = 'A';
	payload[PAYLOAD_LEN - 6 - offset] = 'M';
	payload[PAYLOAD_LEN - 5 - offset] = 'D';
	payload[PAYLOAD_LEN - 4 - offset] = '*';
	payload[PAYLOAD_LEN - 3 - offset] = 'a';
	payload[PAYLOAD_LEN - 2 - offset] = 'm';
	payload[PAYLOAD_LEN - 1 - offset] = 'd';

    uint16_t rows = 0;
    uint16_t cols = 0;
    ELEMENT_TYPE** matrix = generate_pattern_match_matrix( pattern, PATTERN_LEN, PAYLOAD_LEN, &rows, &cols );
    //print_pattern_match_matrix(matrix, rows, cols);

    ELEMENT_TYPE* buffer = generate_pattern_match_array( matrix, rows, cols );

	printf("Generated buffer len: %u\n", (rows * cols) );

	ELEMENT_TYPE* a = payload;
	ELEMENT_TYPE* b = buffer;
	volatile uint32_t c = 0;
	uint32_t d = PATTERN_LEN;    // Singature length
	uint32_t e = rows;           // Parts  (rows in the pattern tables foreach signature)
	uint32_t f = cols;	         // 

	if ( cols != PAYLOAD_LEN )
	{
		printf( "Pattern Matrix column count (%u) does not match PAYLOAD_LEN!\n", cols );
		return -1;
	}

	// Load kernel from file vecExperimentKernel
	FILE *kernelFile;
	char *kernelSource;
	size_t kernelSize;

	kernelFile = fopen("vecExperimentKernel.cl", "r");

	if (!kernelFile) {

		fprintf(stderr, "No file named vecExperimentKernel.cl was found\n");
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
	cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context, deviceID, 0, &ret);

	// Memory buffers for each array - CL_MEM_ALLOC_HOST_PTR
	cl_mem aMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, PAYLOAD_LEN * sizeof(ELEMENT_TYPE), NULL, &ret);
	cl_mem bMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, rows * cols * sizeof(ELEMENT_TYPE), NULL, &ret);
	cl_mem dMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &ret); 
	cl_mem eMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &ret); 
	cl_mem fMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &ret);

	cl_mem cMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), NULL, &ret); 

	// Create program from kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, (const size_t *)&kernelSize, &ret);

	// Build program
	ret = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);

	// Create kernel
	cl_kernel kernel = clCreateKernel(program, "matchVectors", &ret);

	// Set arguments for kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&aMemObj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bMemObj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cMemObj);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&dMemObj);
	ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&eMemObj);
	ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&fMemObj);

	// Execute the kernel

	size_t globalItemSize = PAYLOAD_LEN;
	//size_t localItemSize  = PAYLOAD_LEN;
	cl_uint clDimensions  = 1;

	c = 0;

	// Copy lists to memory buffers
	ret = clEnqueueWriteBuffer(commandQueue, aMemObj, CL_TRUE, 0, PAYLOAD_LEN * sizeof(ELEMENT_TYPE), a, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, rows * cols * sizeof(ELEMENT_TYPE), b, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, dMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&d), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, eMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&e), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, fMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&f), 0, NULL, NULL);

	ret = clEnqueueWriteBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&c), 0, NULL, NULL);

	// Execute kernel
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, clDimensions, NULL, &globalItemSize, NULL, 0, NULL, NULL);
	
	// Get the result
	ret = clEnqueueReadBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(c), (void *)&c, 0, NULL, NULL);

	// Write result
	printf("Rows = %u, Cols = %u\n", rows, cols );
	printf("CL return value: = %u\n", c);
	if ( c == PATTERN_LEN )
	{
		printf("-----> MATCH! \n");
	}

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
	free(payload);
	free(buffer);

	return 0;
}