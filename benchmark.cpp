#include <stdio.h>
#include "dynmatrix.h"
#include <string.h>

#include <CL/cl.h>

#define USE_OPEN_CL ( 1 )

#define MAX_SOURCE_SIZE (0x100000)
#define ELEMENT_TYPE uint8_t

#define PATTERN_LEN ( 7 )

#define MULTIPLIER ( 30 )
#define BUFF_LEN ( 1500 * MULTIPLIER ) // The size should be a configuration option (related to the MTU, MSS, etc)

int main(void)
{
    //
    // This a parsed line from the configuration file with the signatures
    //
    uint8_t pattern[PATTERN_LEN + 1] = { 'a', 'l', 't', 'i', 'n', 'o', 'r' }; // Ugly, but for a reason: +1 for the termination zero of the Non-OpenCL Test

    //
    // This is the packet buffer
    //
    uint16_t i;
    uint8_t packet[BUFF_LEN + 1] = { 0x00 };                                  // Ugly as well, but for the same reason
    for ( i = 0; i < BUFF_LEN; i++ )
        packet[i] = '0';
    
    for ( i = 0; i < PATTERN_LEN; i++ )
        packet[i + ( 44993 )] = pattern[i];

    // Termination zero of the Non-OpenCL Test
    packet[BUFF_LEN]     = '\0';
    pattern[PATTERN_LEN] = '\0';

    //
    // The pattern matrix
    //
    uint16_t rows = 0, columns = 0;
    uint8_t** matrix = generate_pattern_match_matrix(pattern, PATTERN_LEN, BUFF_LEN, &rows, &columns);
    
    #if ( LOG_ENABLED )
    print_pattern_match_matrix(matrix, rows, columns);
    #endif

    //
    // OpenCL Logic
    //
	ELEMENT_TYPE* a = packet;
	volatile uint32_t c = 0;

	// Load kernel from file vecMatchKernel
	FILE *kernelFile;
	char *kernelSource;
	size_t kernelSize;

	kernelFile = fopen("vecMatchKernel.cl", "r");

	if (!kernelFile) {

		fprintf(stderr, "No file named vecMatchKernel.cl was found\n");
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

	// Memory buffers for each array
	cl_mem aMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,  BUFF_LEN * sizeof(ELEMENT_TYPE), NULL, &ret); // Try on iGPU with: CL_MEM_USE_HOST_PTR
	cl_mem bMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,  BUFF_LEN * sizeof(ELEMENT_TYPE), NULL, &ret); // ...
	cl_mem cMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,            sizeof(uint32_t),     NULL, &ret); // .

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

	// Execute the kernel
	size_t globalItemSize = BUFF_LEN;
	cl_uint clDimensions = 1;

    if ( USE_OPEN_CL )
	{
        printf("Using OpenCL...\n");

        for ( int row = 0; row < rows; row++ )
        {  
            uint8_t* b = matrix[row];
            c = 0;

            // Copy lists to memory buffers
            ret = clEnqueueWriteBuffer(commandQueue, aMemObj, CL_TRUE, 0, BUFF_LEN * sizeof(ELEMENT_TYPE), a, 0, NULL, NULL);
            ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, BUFF_LEN * sizeof(ELEMENT_TYPE), b, 0, NULL, NULL);
            ret = clEnqueueWriteBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&c), 0, NULL, NULL);
            
            // Execute kernel
            ret = clEnqueueNDRangeKernel(commandQueue, kernel, clDimensions, NULL, &globalItemSize, NULL, 0, NULL, NULL);
            
            // Get the result
            ret = clEnqueueReadBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(c), (void *)&c, 0, NULL, NULL);

            // Write result
            // printf("Result: c = %u\n", c);
            if ( c == PATTERN_LEN )
            {
                printf("MATCH!\n");
                break;
            }
        }
    }
    else
    {
        printf("Not using OpenCL...\n");

        char* match_index;

        match_index = strstr( (char *)packet, (char *)pattern );
        if ( match_index != NULL )
            printf("MATCH!\n");
    }
    
	ret = clFlush(commandQueue);
	ret = clFinish(commandQueue);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(aMemObj);
	ret = clReleaseMemObject(bMemObj);
	ret = clReleaseMemObject(cMemObj);
	ret = clReleaseContext(context);

    free(matrix);

    return 0;
}