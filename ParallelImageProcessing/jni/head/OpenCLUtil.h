#include "ImageUtil.h"

int openClInit();
void openClFinish();
int processImage(pixel * inImage, pixel * outImage, imagePars pars, unsigned int * kernelTime);
