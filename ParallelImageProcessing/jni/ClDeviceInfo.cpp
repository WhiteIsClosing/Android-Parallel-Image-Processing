#include "head/com_white_parallelimageprocessing_ImageProcessingThread.h"

extern "C" {
extern const char *getClInfoString(void);
JNIEXPORT jstring JNICALL Java_com_white_parallelimageprocessing_ImageProcessingThread_getDeviceInfo(
		JNIEnv* env, jobject thiz) {
	const char* result = getClInfoString();
	return env->NewStringUTF(result);
}
}
