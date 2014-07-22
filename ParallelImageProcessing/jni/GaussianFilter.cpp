#include "head/com_white_parallelimageprocessing_ImageProcessingThread.h"
#include <opencv2/opencv.hpp>
using namespace cv;
JNIEXPORT jstring JNICALL Java_com_white_parallelimageprocessing_ImageProcessingThread_gaussianBlurFilter(
		JNIEnv *env, jobject thiz, jstring image_path, jint itertime) {
	int iter_time = (int) itertime;	  //��������
	char* fileName = (char *) (env)->GetStringUTFChars(image_path, 0);//�����ͼ��·��
	IplImage* inputImg = cvLoadImage(fileName);
	IplImage* outputImg = cvCreateImage(cvGetSize(inputImg), IPL_DEPTH_8U,
			inputImg->nChannels);   //���ͼ��
	cvSmooth(inputImg, outputImg, CV_GAUSSIAN, 27, inputImg->nChannels);
	cvSaveImage("/storage/sdcard0/cpu_gaussian.jpg", outputImg);
	return env->NewStringUTF("Done for gaussian filter!");
}
