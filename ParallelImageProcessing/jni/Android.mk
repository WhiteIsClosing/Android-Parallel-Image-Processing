LOCAL_PATH := $(call my-dir)
LOCAL_PATH_EXT	:= ../extra_libs/
include $(CLEAR_VARS)

include ../../../sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := parallel_image_processing
LOCAL_SRC_FILES := clInfoCore.c ClDeviceInfo.cpp  SRADdenoising.cpp SobelFilter.cpp SHA1.cpp  multMatrix.cpp GaussianFilter.cpp BilateralFilter.cpp #ArgumentsParser.cpp ImageUtil.cpp
LOCAL_LDLIBS	+=-L$(SYSROOT)/usr/lib -llog -ldl -ljnigraphics
LOCAL_LDLIBS 	+= $(LOCAL_PATH_EXT)libOpenCL.so $(LOCAL_PATH_EXT)libFreeImage.so

include $(BUILD_SHARED_LIBRARY) 