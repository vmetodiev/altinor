//
// Prerequisites:
// sudo apt install opencl-headers
// sudo apt install ocl-icd-opencl-dev
//
// Compile with:
// c++ find_match_cl.cpp -o find_match_cl -l OpenCL 
// (or g++)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)
#define ELEMENT_TYPE uint8_t
#define LEN 10
#define PATTERN_LEN 3
#define TEMP_ROWS 3

int main(int argc, char ** argv) {
	
	int SIZE = LEN;

	ELEMENT_TYPE string[LEN] =
        { 'h', 'e', 'l', 'l', 'o', 'a', '5', 'b', 'j', '8' }; // This my string template
   
    ELEMENT_TYPE matrix[PATTERN_LEN][LEN] =
    {
        { 'X', 'X', 'X', 'a', '5', 'b', 'X', 'X', 'X', 'X' }, // #1 will not match
        { 'X', 'X', 'X', 'X', 'a', 'a', '5', 'X', 'X', 'X' }, // #2 will not match
        { 'X', 'X', 'X', 'X', 'X', 'a', '5', 'b', 'X', 'X' }  // #3 will MATCH
    };

	ELEMENT_TYPE* a = string;
	ELEMENT_TYPE* b = nullptr;
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
	cl_mem aMemObj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * sizeof(ELEMENT_TYPE), NULL, &ret);
	cl_mem bMemObj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * sizeof(ELEMENT_TYPE), NULL, &ret);
	cl_mem cMemObj = clCreateBuffer(context, CL_MEM_READ_WRITE,       sizeof(uint32_t), NULL, &ret); 

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

	size_t globalItemSize = SIZE;
	size_t localItemSize = SIZE;
	cl_uint clDimensions = 1;


	for ( int row = 0; row < TEMP_ROWS; row++ )
    {  
		uint8_t* b = matrix[row];
		c = 0;

		// Copy lists to memory buffers
		ret = clEnqueueWriteBuffer(commandQueue, aMemObj, CL_TRUE, 0, SIZE * sizeof(ELEMENT_TYPE), a, 0, NULL, NULL);
		ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, SIZE * sizeof(ELEMENT_TYPE), b, 0, NULL, NULL);
		ret = clEnqueueWriteBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&c), 0, NULL, NULL);
		
		// Execute kernel
		ret = clEnqueueNDRangeKernel(commandQueue, kernel, clDimensions, NULL, &globalItemSize, NULL, 0, NULL, NULL);
		
		// Get the result
		ret = clEnqueueReadBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(c), (void *)&c, 0, NULL, NULL);

		// Write result
		printf("Result: c = %u\n", c);
		if ( c == PATTERN_LEN )
		{
			printf("MATCH! \n");
			break;
		}
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
	
	return 0;
}