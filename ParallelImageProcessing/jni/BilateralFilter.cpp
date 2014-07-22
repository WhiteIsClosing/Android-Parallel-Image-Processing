#include "head/com_white_parallelimageprocessing_ImageProcessingThread.h"
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <CL/cl.hpp>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace cv;

#define CLAMP(x,ll,ul) (((x)<(ll)) ? (ll) : (((x) >(ul)) ? (ul) : (x)))

#define oneOver255 0.00392156862745f
#define sigmaDomain 3.0f
#define sigmaRange  0.2f
#define filterWidth 2

#include <android/log.h>
#define app_name "Bilateral"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, app_name, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, app_name, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, app_name, __VA_ARGS__))

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define __CL_ENABLE_EXCEPTIONS

inline std::string loadProgram(std::string input) {
	std::ifstream stream(input.c_str());
	if (!stream.is_open()) {
		LOGE("Cannot open input file\n");
		exit(1);
	}
	return std::string(std::istreambuf_iterator<char>(stream),
			(std::istreambuf_iterator<char>()));
}

void openCLNR(unsigned char* bufIn, unsigned char* bufOut, int* info,
		int iter) {

	LOGI("\n\nStart openCLNR (i.e., OpenCL on the GPU)");
	clock_t startTimer1, stopTimer1;
	startTimer1 = clock();
	int width = info[0];
	int height = info[1];
	unsigned int imageSize = width * height * 4 * sizeof(cl_uchar);

	cl_int err = CL_SUCCESS;

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.size() == 0) {
		std::cout << "Platform size 0\n";
		return;
	}

	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,
			(cl_context_properties) (platforms[0])(), 0 };
	cl::Context context(CL_DEVICE_TYPE_GPU, properties);

	std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
	cl::CommandQueue queue(context, devices[0], 0, &err);

	std::string kernelSource = loadProgram(
			"/storage/sdcard0/bilateralKernel.cl");

	cl::Program::Sources source(1,
			std::make_pair(kernelSource.c_str(), kernelSource.length() + 1));
	cl::Program program(context, source);
	const char *options = "-cl-fast-relaxed-math";
	program.build(devices, options);

	cl::Kernel kernel(program, "bilateralFilterKernel", &err);

	cl::Buffer bufferIn = cl::Buffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, imageSize, (void *) &bufIn[0],
			&err);
	cl::Buffer bufferOut = cl::Buffer(context,
	CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, imageSize, (void *) &bufOut[0],
			&err);
	cl::Event event;

	if (iter % 2 == 0) {
		kernel.setArg(2, width);
		kernel.setArg(3, height);

		for (int x = 0; x < iter / 2; x++) {
			//one time
			kernel.setArg(0, bufferIn);
			kernel.setArg(1, bufferOut);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange,
					cl::NDRange(width, height), cl::NullRange, NULL, &event);
			//2nd time
			kernel.setArg(0, bufferOut);
			kernel.setArg(1, bufferIn);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange,
					cl::NDRange(width, height), cl::NullRange, NULL, &event);
		}
	} else {

		for (int x = 0; x < iter / 2; x++) {
			//one time
			kernel.setArg(0, bufferIn);
			kernel.setArg(1, bufferOut);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange,
					cl::NDRange(width, height), cl::NullRange, NULL, &event);
			//2nd time
			kernel.setArg(0, bufferOut);
			kernel.setArg(1, bufferIn);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange,
					cl::NDRange(width, height), cl::NullRange, NULL, &event);
		}
		//another time
		kernel.setArg(0, bufferOut);
		kernel.setArg(1, bufferIn);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange,
				cl::NDRange(width, height), cl::NullRange, NULL, &event);
	}
	queue.finish();

	stopTimer1 = clock();
	double elapse = 1000.0 * (double) (stopTimer1 - startTimer1)
			/ (double) CLOCKS_PER_SEC;
	info[2] = (int) elapse;
	LOGI("OpenCL code on the GPU took %g ms\n\n",
			1000.0 * (double )(stopTimer1 - startTimer1)
					/ (double )CLOCKS_PER_SEC);

	queue.enqueueReadBuffer(bufferOut, CL_TRUE, 0, imageSize, bufOut);

	return;
}

void bilateralFilter(float *srcPixels, float* dstPixels, const int width,
		const int height) {
	//loop over the image
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			if ((x >= filterWidth) && (x < (width - filterWidth)) && //avoid reading outside of buffer
					(y >= filterWidth) && (y < (height - filterWidth))) {
				int centerIndex = 4 * (y * width + x);
				float centerPixel[4] = { srcPixels[centerIndex],
						srcPixels[centerIndex + 1], srcPixels[centerIndex + 2],
						srcPixels[centerIndex + 3] };
				float sum4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				float normalizeCoeff = 0.0f;

				for (int yy = -filterWidth; yy <= filterWidth; yy++) {
					for (int xx = -filterWidth; xx <= filterWidth; xx++) {
						int thisIndex = 4 * ((y + yy) * width + (x + xx));
						float currentPixel[4] = { srcPixels[thisIndex],
								srcPixels[thisIndex + 1], srcPixels[thisIndex
										+ 2], srcPixels[thisIndex + 4] };
						float domainDistance = sqrt(
								(float) (xx) * (float) (xx)
										+ (float) (yy) * (float) (yy));
						float domainWeight = exp(
								-0.5f
										* pow((domainDistance / sigmaDomain),
												2.0f));

						float rangeDistance = 0.0f;
						for (int c = 0; c < 3; ++c)
							rangeDistance += (currentPixel[c] - centerPixel[c])
									* (currentPixel[c] - centerPixel[c]);

						rangeDistance = sqrt(rangeDistance);
						float rangeWeight = exp(
								-0.5f
										* pow((rangeDistance / sigmaRange),
												2.0f));

						float totalWeight = domainWeight * rangeWeight;
						normalizeCoeff += totalWeight;

						for (int c = 0; c < 4; c++)
							sum4[c] += totalWeight * currentPixel[c];
					}
				}

				for (int c = 0; c < 3; c++)
					dstPixels[centerIndex + c] = CLAMP(
							(sum4[c] / normalizeCoeff), 0.0f, 1.0f);
				dstPixels[centerIndex + 3] = 1.0f; // set alpha to fully opaque
			}
		}
	}

}

void refNR(unsigned char* bufIn, unsigned char* bufOut, int* info, int iter) {
	int width, height;
	clock_t startTimer, stopTimer;

	width = info[0];
	height = info[1];

	float* srcPixels = new float[width * height * 4];
	float* dstPixels = new float[width * height * 4];

	//convert input image buffer to float
	int index1 = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			for (int c = 0; c < 4; c++) {
				srcPixels[index1] = oneOver255 * (float) bufIn[index1];
				//dstPixels[index1] = 0;// reset dstPixels
				index1++;
			}
		}
	}

	startTimer = clock();
	//run the bilateral filter three times to get a stronger effect
	if (iter % 2 == 0) {
		for (int i = 0; i < iter / 2; i++) {
			bilateralFilter(srcPixels, dstPixels, width, height);
			bilateralFilter(dstPixels, srcPixels, width, height);
		}
	} else {
		for (int i = 0; i < iter / 2; i++) {
			bilateralFilter(srcPixels, dstPixels, width, height);
			bilateralFilter(dstPixels, srcPixels, width, height);
		}
		bilateralFilter(srcPixels, dstPixels, width, height);
	}
	stopTimer = clock();
	double elapse = (double) 1000 * (stopTimer - startTimer)
			/ (double) CLOCKS_PER_SEC;
	info[2] = (int) elapse;

	//LOGI("C++ code on the CPU took %g ms\n\n", 1000.0* (double)(stopTimer - startTimer)/(double)CLOCKS_PER_SEC) ;

	//convert to uchar and write to bufOut
	index1 = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			for (int c = 0; c < 3; c++) {
				bufOut[index1] = (unsigned char) CLAMP(
						255.0f * dstPixels[index1], 0.0f, 255.0f);
				index1++;
			}
			bufOut[index1++] = 255; // set alpha to fully opaque
		}
	}

	delete srcPixels;
	delete dstPixels;

	return;
}

extern "C" {
JNIEXPORT jstring JNICALL Java_com_white_parallelimageprocessing_ImageProcessingThread_bilateralfilter(
		JNIEnv *env, jobject thiz, jobject bitmapIn, jobject bitmapCPU,
		jobject bitmapGPU, jint width, jint height, jint iterTime) {

	int iter_time = (int) iterTime;	  //迭代次数
	int WIDTH = (int) width;
	int HEIGHT = (int) height;
	void* bi;
	void* boCPU;
	void* boGPU;
	AndroidBitmap_lockPixels(env, bitmapIn, &bi);
	AndroidBitmap_lockPixels(env, bitmapCPU, &boCPU);
	AndroidBitmap_lockPixels(env, bitmapGPU, &boGPU);
	int i[3], j[3];
	i[0] = WIDTH;
	i[1] = HEIGHT;
	j[0] = WIDTH;
	j[1] = HEIGHT;
	refNR((unsigned char *) bi, (unsigned char *) boCPU, (int *) i, iter_time);
	openCLNR((unsigned char *) bi, (unsigned char *) boGPU, (int *) j,
			iter_time);
	//下面是opencl处理的方法
	AndroidBitmap_unlockPixels(env, bitmapIn);
	AndroidBitmap_unlockPixels(env, bitmapCPU);
	AndroidBitmap_unlockPixels(env, bitmapGPU);

	char result_string[128] = { 0 };
	char cpuResult[16];
	char gpuResult[16];
	sprintf(cpuResult, "%d", i[2]);
	sprintf(gpuResult, "%d", j[2]);
	strcat(result_string, cpuResult);
	strcat(result_string, "/");
	strcat(result_string, gpuResult);
	LOGI("cpu time = %d\n", i[2]);
	LOGI("gpu time = %d\n", j[2]);
	return env->NewStringUTF(result_string);
}
}
