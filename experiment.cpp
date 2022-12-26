//
// Prerequisites:
// sudo apt install opencl-headers
// sudo apt install ocl-icd-opencl-dev
//
// Compile with:
// c++ experiment.cpp -o experiment -l OpenCL 
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

	ELEMENT_TYPE string[SIZE] =
        { 
			'A', 'M', 'D', 'M', 'D', 'D', '2', '3', '4', '5' 
		}; // Packet (payload)
   
    ELEMENT_TYPE buffer[4 * SIZE] =
        { 
			'A', 'M', 'D', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 
			'X', 'A', 'M', 'D', 'x', 'X', 'X', 'X', 'X', 'X',
			'X', 'X', 'A', 'M', 'D', 'X', 'X', 'X', 'X', 'X', 
			'X', 'X', 'X', 'A', 'M', 'D', 'X', 'X', 'X', 'X'
		}; // Signature table

	ELEMENT_TYPE* a = string;
	ELEMENT_TYPE* b = buffer;
	volatile uint32_t c = 0;
	uint32_t d = SIZE; // Length (or the MTU/MSS/Packet Payload size)
	uint32_t e = 4;    // Parts  (rows in the pattern tables foreach signature)
	uint32_t f = 3;    // Pattern (signature) length

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

	// Memory buffers for each array
	cl_mem aMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, 	   SIZE * sizeof(ELEMENT_TYPE), NULL, &ret);
	cl_mem bMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, e * SIZE * sizeof(ELEMENT_TYPE), NULL, &ret);
	cl_mem dMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,        	  sizeof(uint32_t), NULL, &ret); 
	cl_mem eMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,            sizeof(uint32_t), NULL, &ret); 
	cl_mem fMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,            sizeof(uint32_t), NULL, &ret);

	cl_mem cMemObj = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,       	  sizeof(uint32_t), NULL, &ret); 

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

	size_t globalItemSize = SIZE;
	size_t localItemSize = SIZE;
	cl_uint clDimensions = 1;

	c = 0;

	// Copy lists to memory buffers
	ret = clEnqueueWriteBuffer(commandQueue, aMemObj, CL_TRUE, 0, SIZE * sizeof(ELEMENT_TYPE), a, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, 4 * SIZE * sizeof(ELEMENT_TYPE), b, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, dMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&d), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, eMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&e), 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, fMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&f), 0, NULL, NULL);

	ret = clEnqueueWriteBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(uint32_t), (const void*)(&c), 0, NULL, NULL);

	// Execute kernel
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, clDimensions, NULL, &globalItemSize, &localItemSize, 0, NULL, NULL);
	
	// Get the result
	ret = clEnqueueReadBuffer(commandQueue, cMemObj, CL_TRUE, 0, sizeof(c), (void *)&c, 0, NULL, NULL);

	// Write result
	printf("Result: c = %u\n", c);
	if ( c == 255 )
	{
		printf("MATCH! \n");
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