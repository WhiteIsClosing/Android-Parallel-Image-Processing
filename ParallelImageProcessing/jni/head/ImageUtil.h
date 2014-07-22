#include <cstdlib>
#include <cstdio>
#include <CL/cl.h>
#include "FreeImage.h"

#define IMAGEUTIL_UNKNOWN_FORMAT -1
#define IMAGEUTIL_LOAD_IMAGE_ERROR -2
#define IMAGEUTIL_UNSUPPORT_FORMAT -3
#define IMAGEUTIL_SUCCESS 1


typedef cl_uchar4 pixel;

struct imagePars {
	cl_uint width;
	cl_uint height;
	cl_uint imageSize;
	cl_uint channelNum;
};

class ImageUtil
{
public:
	ImageUtil(void);
	ImageUtil(char *filename);
	~ImageUtil(void);

//public:
private:
	FIBITMAP *src;
	//FIBITMAP *src4;
	FIBITMAP *dest;
	//FIBITMAP *dest4;
	pixel *srcData;
	//pixel *srcData4;
	pixel *destData;
	//pixel *destData4;
	FREE_IMAGE_FORMAT imageType;
	unsigned int width;
	unsigned int height;
	unsigned int pixelLength;

public:
	int getWidth();
	int getHeight();
	unsigned int getChannelNum();
	int loadImage(char *filename);
	int saveImage(char *filename);
	pixel ** getSource();
	pixel ** getDestination();
};
