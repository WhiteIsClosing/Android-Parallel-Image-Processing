struct imagePathPars{
	int argc;
	int imageNum;
	char ** imagePath;
	char ** imageInputPath;
	char ** imageOutputPath;
	char ** imageInputName;
	char ** imageOutputName;
};

imagePathPars ImagePathInit(int argc, char *argv[],int * verbose);
void imagePathFinish(imagePathPars * imgPath);
void printHelp();
void printVersion();
