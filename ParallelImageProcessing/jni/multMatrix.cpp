#include<CL/cl.h>
#include "head/com_white_parallelimageprocessing_ImageProcessingThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <android/log.h>

using namespace std;

const char* simple_source =
		"__kernel                                           \n"
				"void simpleMultiply(__global float* outputC,       \n"
				"int widthA,                                        \n"
				"int heightA,                                       \n"
				"int widthB,                                        \n"
				"int heightB,                                       \n"
				"__global float* inputA,                            \n"
				"__global float* inputB)                            \n"
				"{                                                  \n"
				"int row=get_global_id(1);                          \n"
				"int col=get_global_id(0);                          \n"
				"float sum=0.0f;                                    \n"
				"for(int i=0;i<widthA;i++){                         \n"
				"sum+=inputA[row*widthA+i]*inputB[i*widthB+col];    \n"
				"}                                                  \n"
				"outputC[row*widthB+col]=sum;                       \n"
				"}";
const char* muti_source =
		"#define BLOCKSIZE 8													\n"
				"__kernel void multMatrix(__global float *mO,							\n"
				"                         __global float *mA,							\n"
				"                         __global float *mB,							\n"
				"                      	  uint widthA, uint widthB)						\n"
				"{																		\n"
				"	uint lx = get_local_id(0);											\n"
				"	uint ly = get_local_id(1);											\n"
				"	int gx = get_group_id(0);											\n"
				"	int gy = get_group_id(1);											\n"
				"	uint iSubA = BLOCKSIZE * gy * widthA;								\n"
				"	uint iSubB = BLOCKSIZE * gx;										\n"
				"	int n = get_num_groups(0);											\n"
				"	float sum = 0;														\n"
				"	for(int i=0; i< n;i++)												\n"
				"	{																	\n"
				"	  __local float tA[BLOCKSIZE][BLOCKSIZE];							\n"
				"	  __local float tB[BLOCKSIZE][BLOCKSIZE];							\n"
				"	  tA[ly][lx] = mA[ly*widthA + lx + (iSubA + i* BLOCKSIZE)];			\n"
				"	  tB[ly][lx] = mB[ly*widthB + lx + (iSubB + i* BLOCKSIZE * widthB)];\n"
				"	  barrier(CLK_LOCAL_MEM_FENCE);										\n"
				"	  for(int k=0; k<BLOCKSIZE; k++){									\n"
				"	    sum += tA[ly][k] * tB[k][lx];									\n"
				"	  }																	\n"
				"	}																	\n"
				"	int globalIdx=get_global_id(0);										\n"
				"	int globalIdy=get_global_id(1);										\n"
				"	mO[globalIdy * widthA + globalIdx] = sum;							\n"
				"}";

void simpleMultiply(int len);
clock_t matrix_start, matrix_finish;
cl_int ciErrNum;
float*A = NULL;
float*B = NULL;
float*C = NULL;

JNIEXPORT jstring JNICALL Java_com_white_parallelimageprocessing_ImageProcessingThread_multmatrix(
		JNIEnv *env, jobject thiz, jint len, jint itertime) {
	int length = (int) len, iter_time = (int) itertime;
	double GPU_time, CPU_time;

	for (int i = 0; i < iter_time; i++)
		simpleMultiply(length);

	GPU_time = (double) (matrix_finish - matrix_start) / CLOCKS_PER_SEC;

	char result[128];
	char gpu_result[16];
	sprintf(gpu_result, "%f", GPU_time);
	strcat(result, gpu_result);

	strcat(result, "s");
	return env->NewStringUTF(result);
}

void simpleMultiply(int len) {

	int wA = len, hA = len;
	int wB = len, hB = len;
	int wC = len, hC = len;

	const int elementsA = wA * hA;
	const int elementsB = wB * hB;
	const int elementsC = hA * wB;

	// 计算内存大小
	size_t datasizeA = sizeof(float) * elementsA;
	size_t datasizeB = sizeof(float) * elementsB;
	size_t datasizeC = sizeof(float) * elementsC;
	// 分配内存空间
	A = (float*) malloc(datasizeA);
	B = (float*) malloc(datasizeB);
	C = (float*) malloc(datasizeC);

	//init the data
	for (int i = 0; i < wA * hA; i++)
		A[i] = 1.0;

	for (int i = 0; i < wB * hB; i++)
		B[i] = 1.0;

	matrix_start = clock();
	cl_platform_id platform;
	ciErrNum = clGetPlatformIDs(1, &platform, NULL);

	cl_device_id device;
	ciErrNum = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
	cl_context_properties cps[3] = {
	CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0 };

	cl_context ctx = clCreateContext(cps, 1, &device, NULL, NULL, &ciErrNum);
	cl_command_queue myqueue = clCreateCommandQueue(ctx, device, 0, &ciErrNum);
	cl_mem bufferA = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
			wA * hA * sizeof(float), NULL, &ciErrNum);
	ciErrNum = clEnqueueWriteBuffer(myqueue, bufferA, CL_TRUE, 0,
			wA * hA * sizeof(float), (void*) A, 0, NULL, NULL);
	cl_mem bufferB = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
			wB * hB * sizeof(float), NULL, &ciErrNum);
	ciErrNum = clEnqueueWriteBuffer(myqueue, bufferB, CL_TRUE, 0,
			wB * hB * sizeof(float), (void*) B, 0, NULL, NULL);
	cl_mem bufferC = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY,
			hA * wB * sizeof(float), NULL, &ciErrNum);
	cl_program myprog = clCreateProgramWithSource(ctx, 1,
			(const char**) &muti_source, NULL, &ciErrNum);
	ciErrNum = clBuildProgram(myprog, 0, NULL, NULL, NULL, NULL);
	cl_kernel mykernel = clCreateKernel(myprog, "simpleMultiply", &ciErrNum);

	clSetKernelArg(mykernel, 0, sizeof(cl_mem), (void*) &bufferC);
	clSetKernelArg(mykernel, 1, sizeof(cl_int), (void*) &wA);
	clSetKernelArg(mykernel, 2, sizeof(cl_int), (void*) &hA);
	clSetKernelArg(mykernel, 3, sizeof(cl_int), (void*) &wB);
	clSetKernelArg(mykernel, 4, sizeof(cl_int), (void*) &hB);
	clSetKernelArg(mykernel, 5, sizeof(cl_mem), (void*) &bufferA);
	clSetKernelArg(mykernel, 6, sizeof(cl_mem), (void*) &bufferB);

	size_t localws[2] = { 8, 8 };
	size_t globalws[2] = { wC, hC };

	ciErrNum = clEnqueueNDRangeKernel(myqueue, mykernel, 2, NULL, globalws,
			localws, 0, NULL, NULL);
	ciErrNum = clEnqueueReadBuffer(myqueue, bufferC, CL_TRUE, 0,
			wC * hC * sizeof(float), (void*) C, 0, NULL, NULL);
	matrix_finish = clock();
	clReleaseKernel(mykernel);
	clReleaseProgram(myprog);
	clReleaseCommandQueue(myqueue);
	clReleaseMemObject(bufferA);
	clReleaseMemObject(bufferB);
	clReleaseMemObject(bufferC);
	clReleaseContext(ctx);

	free(A);
	free(B);
	free(C);
}
