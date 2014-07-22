#include <stdio.h>
#include <time.h>
#include <head/SHA1.h>
#include "head/com_white_parallelimageprocessing_ImageProcessingThread.h"

static cl_platform_id platform_id = NULL;
static cl_device_id device_id = NULL;
static cl_uint ret_num_devices;
static cl_uint ret_num_platforms;
static cl_context context;

static cl_int ret;

static char* source_str;
static size_t source_size;

static cl_program program;
static cl_kernel kernel;
static cl_command_queue command_queue;

static cl_mem pinned_saved_keys, pinned_partial_hashes, buffer_out, buffer_keys,
		data_info;
static cl_uint *partial_hashes;
static cl_uint *res_hashes;
static char *saved_plain;
static unsigned int datai[3];
static int have_full_hashes;

static size_t kpc = 4;

static size_t global_work_size = 1;
static size_t local_work_size = 1;
static size_t string_len;

static char key_string[64];

void load_source();
void createDevice();
void createkernel();
void create_clobj();
void crypt_all();
void binarystring(char c);

extern "C" {
JNIEXPORT jstring JNICALL Java_com_white_parallelimageprocessing_ImageProcessingThread_sha1entryption(
		JNIEnv* env, jobject thiz, jstring plain_text, jint iterTime) {

	char* plainText = (char *) (env)->GetStringUTFChars(plain_text, 0);

	clock_t sclock, eclock;
	double GPU_TIME;
	char result[64];

	sha1_init(2048);
	sclock = clock();
	for (int i = 0; i < (int) iterTime; i++)
		sha1_crypt(plainText, result);
	eclock = clock();
	GPU_TIME = (eclock - sclock) / (double) (CLOCKS_PER_SEC);

	char final_result[128];
	char gpu_result[8];

	sprintf(gpu_result, "%.5f", GPU_TIME);

//	strcat(final_result, "GPU time:");
	strcat(final_result, gpu_result);
//	strcat(final_result, "s/");
//	strcat(final_result, key_string);
	strcat(final_result, "/");
	strcat(final_result, result);

	return env->NewStringUTF(final_result);
}
}

void binarystring(char c) {
	int i;

	for (i = 0; i < 8; i++) {
		if (c & 0x80)
			putchar('1');
		else
			putchar('0');
		c <<= 1;
	}
}

void sha1_init(size_t user_kpc) {
	kpc = user_kpc;
	load_source();
	createDevice();
	createkernel();
	create_clobj();
}

void sha1_crypt(char* input, char* output) {
	int i;
	string_len = strlen(input);
	global_work_size = 1;
	datai[0] = SHA1_PLAINTEXT_LENGTH;
	datai[1] = global_work_size;
	datai[2] = string_len;
	memcpy(saved_plain, input, string_len + 1);

	crypt_all();

	for (i = 0; i < SHA1_RESULT_SIZE; i++) {
		sprintf(output + i * 8, "%08x", partial_hashes[i]);

	}
}

void crypt_all() {
	printf("%s\n", saved_plain); //Ã÷ÎÄ£¿
	ret = clEnqueueWriteBuffer(command_queue, data_info, CL_TRUE, 0,
			sizeof(unsigned int) * 3, datai, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, buffer_keys, CL_TRUE, 0,
			SHA1_PLAINTEXT_LENGTH * kpc, saved_plain, 0, NULL, NULL);
	printf("%s\n", buffer_keys); //ÃÜÔ¿

	for (int i = 0; i < strlen((char*) buffer_keys); i++)
		binarystring(((char*) buffer_keys)[i]);
	printf("\n");
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
			&global_work_size, &local_work_size, 0, NULL, NULL);
	ret = clFinish(command_queue);
	// read back partial hashes
	ret = clEnqueueReadBuffer(command_queue, buffer_out, CL_TRUE, 0,
			sizeof(cl_uint) * SHA1_RESULT_SIZE, partial_hashes, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, buffer_keys, CL_TRUE, 0,
				sizeof(cl_uint) * SHA1_PLAINTEXT_LENGTH * kpc, key_string, 0, NULL, NULL);

	have_full_hashes = 0;
}

void load_source() {
	FILE *fp;
	fp = fopen("/storage/sdcard0/sha1.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
}

void create_clobj() {
	pinned_saved_keys = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
			(SHA1_PLAINTEXT_LENGTH) * kpc, NULL, &ret);
	saved_plain = (char*) clEnqueueMapBuffer(command_queue, pinned_saved_keys,
			CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0,
			(SHA1_PLAINTEXT_LENGTH) * kpc, 0, NULL, NULL, &ret);
	memset(saved_plain, 0, SHA1_PLAINTEXT_LENGTH * kpc);
	res_hashes = (cl_uint *) malloc(sizeof(cl_uint) * SHA1_RESULT_SIZE);
	memset(res_hashes, 0, sizeof(cl_uint) * SHA1_RESULT_SIZE);
	pinned_partial_hashes = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
			sizeof(cl_uint) * SHA1_RESULT_SIZE, NULL, &ret);
	partial_hashes = (cl_uint *) clEnqueueMapBuffer(command_queue,
			pinned_partial_hashes, CL_TRUE, CL_MAP_READ, 0,
			sizeof(cl_uint) * SHA1_RESULT_SIZE, 0, NULL, NULL, &ret);
	memset(partial_hashes, 0, sizeof(cl_uint) * SHA1_RESULT_SIZE);

	buffer_keys = clCreateBuffer(context, CL_MEM_READ_ONLY,
			(SHA1_PLAINTEXT_LENGTH) * kpc, NULL, &ret);
	buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
			sizeof(cl_uint) * SHA1_RESULT_SIZE, NULL, &ret);
	data_info = clCreateBuffer(context, CL_MEM_READ_ONLY,
			sizeof(unsigned int) * 3, NULL, &ret);

	clSetKernelArg(kernel, 0, sizeof(data_info), (void *) &data_info);
	clSetKernelArg(kernel, 1, sizeof(buffer_keys), (void *) &buffer_keys);
	clSetKernelArg(kernel, 2, sizeof(buffer_out), (void *) &buffer_out);
}

void createDevice() {
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id,
			&ret_num_devices);
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
}

void createkernel() {
	program = clCreateProgramWithSource(context, 1, (const char **) &source_str,
			(const size_t *) &source_size, &ret);
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	kernel = clCreateKernel(program, "sha1_crypt_kernel", &ret);
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
}

