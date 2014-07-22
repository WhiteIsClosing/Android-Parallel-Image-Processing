#include "head/ArgumentsParser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int verbose = false;

imagePathPars ImagePathInit(int argc, char *argv[],int * verbose)
{
	imagePathPars par = {argc,0,
		(char **)malloc(sizeof(char *) * argc),
		(char **)malloc(sizeof(char *) * argc),
		(char **)malloc(sizeof(char *) * argc),
		(char **)malloc(sizeof(char *) * argc),
		(char **)malloc(sizeof(char *) * argc)};
	if (argc == 1)
	{
		printHelp();
	}
	else
	{
		//指针没学好的结果....
		//*par.imagePath = (char *)malloc(sizeof(char *) * argc);
		//*par.imageInputPath = (char *)malloc(sizeof(char *) * argc);
		//*par.imageOutputPath = (char *)malloc(sizeof(char *) * argc);
		//*par.imageInputName = (char *)malloc(sizeof(char *) * argc);
		//*par.imageOutputName = (char *)malloc(sizeof(char *) * argc);
		char inputPath[255] = "\0";
		char inputName[255] = "\0";
		char outputPath[255] = "\0";
		char outputName[255] = "\0";
		char exName[255] = "\0";
		int j = 0;
		int posSplit = -1;
		int posDot = -1;

		for (int i = 1; i < argc; i++)
		{
			if(strcmp("--version",argv[i]) == 0)
			{
				//输出版本信息
				if (i != 1) exit(0);
				printVersion();
				par.imageNum = -1;
				break;
			}
			else if(strcmp("-h",argv[i]) == 0 || strcmp("--help",argv[i]) == 0)
			{
				//输出help
				if (i != 1) exit(0);
				printHelp();
				par.imageNum = -1;
				break;
			}
			else if (strcmp("-v",argv[i]) == 0 || strcmp("--verbose",argv[i]) == 0)
			{
				//屏幕输出
				*verbose = true;
			}
			else
			{
				//添加路径
				par.imagePath[par.imageNum] = argv[i];
				//从path中获取最后一个路径分隔符和.并将内容copy到inputPath中
				j = 0;
				posSplit = -1;
				posDot = -1;
				while(par.imagePath[par.imageNum][j])
				{
					inputPath[j] = par.imagePath[par.imageNum][j];
					//Windows系统分隔符多一个":"
#ifdef WIN32
					if (inputPath[j] == '\\' || inputPath[j] == '/' || inputPath[j] == ':')
#else
					if (inputPath[j] == '\\' || inputPath[j] == '/')
#endif
					{
						posSplit = j;
					}
					else if (inputPath[j] == '.')
					{
						posDot = j;
					}
					j++;
				}
				inputPath[j] = '\0';
				if(posSplit != -1)
				{
					//设置inputName
					strcpy(inputName,&(inputPath[posSplit + 1]));
					par.imageInputName[par.imageNum] = (char *)malloc(strlen(inputName) + 1);
					strcpy(par.imageInputName[par.imageNum],inputName);
					//设置inputPath
					inputPath[posSplit + 1] = '\0';
					par.imageInputPath[par.imageNum] = (char *)malloc(posSplit + strlen(inputName) + 2);
					strcpy(par.imageInputPath[par.imageNum],inputPath);
					strcat(par.imageInputPath[par.imageNum],inputName);
					//设置outputPath
					par.imageOutputPath[par.imageNum] = (char *)malloc(posSplit + strlen("_output") + 2);
					strcpy(outputPath,inputPath);
					strcpy(par.imageOutputPath[par.imageNum],outputPath);
				}
				else
				{
					//设置inputName
					strcpy(inputName,inputPath);
					par.imageInputName[par.imageNum] = (char *)malloc(strlen(inputName) + 1);
					strcpy(par.imageInputName[par.imageNum],inputName);
					//设置inputPath
					inputPath[0] = '\0';
					par.imageInputPath[par.imageNum] = (char *)malloc(strlen(inputName) + 1);
					strcpy(par.imageInputPath[par.imageNum],inputName);
					//设置outputPath
					par.imageOutputPath[par.imageNum] = (char *)malloc(strlen(inputName) + strlen("_output") + 1);
					strcpy(outputPath,inputPath);
					strcpy(par.imageOutputPath[par.imageNum],outputPath);
				}

				//设置outputName为balabala_output.扩展名，没有扩展名则balabala_output
				par.imageOutputName[par.imageNum] = (char *)malloc(strlen(inputName) + strlen("_output") + 1);
				posDot -= posSplit; //修正dot的位置
				if(posDot > 0)	//有扩展名
				{
					//提取扩展名
					strcpy(exName,&inputName[posDot]);
					strcpy(outputName,inputName);
					outputName[posDot == 0 ? posDot : posDot - 1] = '\0';
					strcat(outputName,"_output.");
					strcat(outputName,exName);
				}
				else	//无扩展名
				{
					strcpy(outputName,inputName);
					strcat(outputName,"_output");
				}
				strcpy(par.imageOutputName[par.imageNum],outputName);
				strcat(par.imageOutputPath[par.imageNum],outputName);
				//           \bin\a.bmp   split = 4  dot = 6    dot - split > 0
				par.imageNum++;
			}
		}
	}
	return par;
}

void printHelp()
{
	printf("help information\n");
	return;
}

void printVersion()
{
	printf("version information\n");
	return;
}

void imagePathFinish(imagePathPars * imgPath)
{
	for(int i = 1; i < imgPath->imageNum; i++)
	{
		free(imgPath->imageInputName[i]);
		free(imgPath->imageInputPath[i]);
		free(imgPath->imageOutputName[i]);
		free(imgPath->imageOutputPath[i]);
		//free(imgPath->imagePath[i]);
	}
	return;
}
