#include "head/FreeImage.h"
#include "head/ImageUtil.h"

ImageUtil::ImageUtil(void)
{
	FreeImage_Initialise(FALSE);

	src = NULL;
	dest = NULL;

	srcData = NULL;
	destData = NULL;

	width = 0;
	height = 0;
	pixelLength = 0;
}
//
//ImageUtil::ImageUtil(char *filename)
//{
//	ImageUtil();
//	loadImage(filename);
//}
//
//ImageUtil::~ImageUtil(void)
//{
//	if(src)
//	{
//		FreeImage_Unload(src);
//	}
//
//	if(dest)
//	{
//		FreeImage_Unload(dest);
//	}
//
//	FreeImage_DeInitialise();
//}
//
//int ImageUtil::loadImage(char *filename)
//{
//	imageType = FIF_UNKNOWN;
//
//	imageType = FreeImage_GetFileType(filename,0);
//
//	// 设法根据图片文件类型猜测图像格式
//	if(imageType == FIF_UNKNOWN)
//	{
//		imageType = FreeImage_GetFIFFromFilename(filename);
//	}
//
//	if(imageType == FIF_UNKNOWN)
//	{
//		return IMAGEUTIL_UNKNOWN_FORMAT;
//	}
//
//	// 检测是否FreeImage是否支持该文件格式
//	if((imageType != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(imageType))
//	{
//		src = FreeImage_Load(imageType,filename,0);
//
//		if(!src)
//		{
//			return IMAGEUTIL_LOAD_IMAGE_ERROR;
//		}
//	}
//	else
//	{
//		return IMAGEUTIL_UNSUPPORT_FORMAT;
//	}
//
//	pixelLength = FreeImage_GetBPP(src);
//	width = FreeImage_GetWidth(src);
//	height = FreeImage_GetHeight(src);
//
//	//位图处理
//	src = FreeImage_ConvertTo32Bits(src);
//
//	srcData = (pixel *)FreeImage_GetBits(src);
//
//	//根据高度和宽度生成dest位图和destData
//	dest = FreeImage_Allocate(width,height,pixelLength);
//	dest = FreeImage_ConvertTo32Bits(dest);
//	//dest = FreeImage_Clone(src);
//	destData = (pixel *)FreeImage_GetBits(dest);
//	return IMAGEUTIL_SUCCESS;
//}
//
//int ImageUtil::saveImage(char *filename)
//{
//	switch (pixelLength)
//	{
//	case 24:
//		dest = FreeImage_ConvertTo24Bits(dest);
//		break;
//	case 8:
//		dest = FreeImage_ConvertTo8Bits(dest);
//		break;
//	case 32:
//		break;
//	default:
//		break;
//	}
//	FreeImage_Save(imageType,dest,filename,0);
//	return IMAGEUTIL_SUCCESS;
//}
//
//pixel ** ImageUtil::getSource()
//{
//	return &srcData;
//}
//
//pixel ** ImageUtil::getDestination()
//{
//	return &destData;
//}
//
//int ImageUtil::getWidth()
//{
//	return width;
//}
//
//int ImageUtil::getHeight()
//{
//	return height;
//}
//
//unsigned int ImageUtil::getChannelNum()
//{
//	return pixelLength / 8;
//}
