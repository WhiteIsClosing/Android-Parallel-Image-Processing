

#include <CL/cl.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "android/log.h"

#define LOGV(F, ...) __android_log_print(ANDROID_LOG_VERBOSE, "aparapi", F, ##__VA_ARGS__)
#define LOGD(F, ...) __android_log_print(ANDROID_LOG_DEBUG, "aparapi", F, ##__VA_ARGS__)
#define LOGW(F, ...) __android_log_print(ANDROID_LOG_WARN, "aparapi", F, ##__VA_ARGS__)
#define LOGE(F, ...) __android_log_print(ANDROID_LOG_ERROR, "aparapi", F, ##__VA_ARGS__)

#define GET_SIZET(CL_D,str) { \
   size_t val; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   if (err !=  CL_SUCCESS){\
  LOGW("GET_SIZET " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGV("GET_SIZET " #CL_D " --> %s val:%d\n",str,val);\
  index += sprintf(&result[index], str, (unsigned int)val); \
   }\
}

#define GET_STRING(CL_D,str,size) { \
   char val[size]; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   /*checkErr(err, "clGetDeviceInfo(" #CL_D ")");*/ \
   /*printf(str, val); */\
   if (err !=  CL_SUCCESS){\
  LOGW("GET_STRING " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGV("GET_STRING " #CL_D " --> %s val:%s\n",str,val);\
  index += sprintf(&result[index], str, val); \
   }\
}

#define GET_UINT(CL_D,str) { \
   cl_uint val; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   /* checkErr(err, "clGetDeviceInfo(" #CL_D ")"); */\
   /*printf(str, val); */\
   if (err !=  CL_SUCCESS){\
  LOGW("GET_UINT " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGV("GET_UINT " #CL_D " --> %s val:0x%x\n",str,val);\
  index += sprintf(&result[index], str, val); \
   }\
}

#define GET_ULONG(CL_D,str) { \
   cl_ulong val; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   /* checkErr(err, "clGetDeviceInfo(" #CL_D ")"); */\
   /* printf(str, val); */\
   if (err !=  CL_SUCCESS){\
  LOGW("GET_ULONG " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGV("GET_ULONG " #CL_D " --> %s val:0x%x\n",str,val);\
  index += sprintf(&result[index], str, val); \
   }\
}

#define GET_BOOL(CL_D,str) { \
   cl_bool val; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   /* checkErr(err, "clGetDeviceInfo(" #CL_D ")"); */\
   /* printf(str, (val == CL_TRUE ? "Yes" : "No")); */\
   if (err !=  CL_SUCCESS){\
  LOGW("GET_BOOL " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGV("GET_BOOL " #CL_D " --> %s val:0x%x\n",str,val);\
  index += sprintf(&result[index], str, (val == CL_TRUE ? "True" : "False")); \
   }\
}

#define GET_BOOL_CUSTOM(CL_D,str,t,f) { \
   cl_bool val; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   /* checkErr(err, "clGetDeviceInfo(" #CL_D ")"); */\
   /* printf(str, (val == CL_TRUE ? t : f)); */\
   if (err !=  CL_SUCCESS){\
  LOGW("GET_BOOL_CUSTOM " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGV("GET_BOOL_CUSTOM " #CL_D " --> %s val:0x%x\n",str,val);\
  index += sprintf(&result[index], str, (val == CL_TRUE ? "True" : "False")); \
   }\
}

#define GET_BITSET_AND(TYPE,CL_D,test,str) { \
   TYPE val; \
   err = rclGetDeviceInfo(devices[j], CL_D, sizeof(val), &val, NULL); \
   /* checkErr(err, "clGetDeviceInfo(" #CL_D ")"); */\
   /* printf(str, ((val & test) == CL_TRUE ? "Yes" : "No")); */ \
   if (err !=  CL_SUCCESS){\
  LOGW("GET_BITSET_AND " #CL_D " --> %s err:%d\n",str,err);\
  index += sprintf(&result[index], "Error:" #CL_D ":%d\n", err); \
   }else{\
  LOGW("GET_BITSET_AND " #CL_D " --> %s val:0x%x\n",str,val);\
  index += sprintf(&result[index], str, (val == CL_TRUE ? "True" : "False")); \
   }\
}

/*
 static inline void checkErr(cl_int err, const char * name) {
 if (err != CL_SUCCESS) {
 fprintf(stderr, "ERROR: %s (%d)\n", name, err);
 exit(1);
 }
 }

 #define IND_0 ""
 #define IND_1 " "
 #define IND_2 "  "
 #define IND_3 "   "
 #define IND_4 "    "

 //static const char** INDENTS= { IND_0, IND_1, IND_2, IND_3, IND_4};
 */
static int loadedCL;

void *getCLHandle() {
	void *res = NULL;
	res = dlopen("/system/lib/libOpenCL.so", RTLD_LAZY);
//#if 0
	if(res==NULL) {
		res = dlopen("/system/vendor/lib/egl/libGLES_mali.so",RTLD_LAZY);
	}
	if(res==NULL) {
		res = dlopen("/system/vendor/lib/libOpenCL.so",RTLD_LAZY);
	}
	if(res==NULL) {
		res = dlopen("/system/lib/libllvm-a3xx.so",RTLD_LAZY);
	}
	/*  if(res==NULL){
	 res = dlopen("/home/rahul/stream/sdk2.7/lib/x86_64/libOpenCL.so",RTLD_LAZY);
	 }*/
//#endif
	if (res == NULL)
		printf("Could not open library :(\n");
	else
		printf("loaded some library\n");
	return res;
}

cl_int (*rclGetPlatformIDs)(cl_uint /* num_entries */,
		cl_platform_id * /* platforms */, cl_uint * /* num_platforms */);

cl_int (*rclGetPlatformInfo)(cl_platform_id /* platform */,
		cl_platform_info /* param_name */, size_t /* param_value_size */,
		void * /* param_value */, size_t * /* param_value_size_ret */);

cl_int (*rclGetDeviceIDs)(cl_platform_id /* platform */,
		cl_device_type /* device_type */, cl_uint /* num_entries */,
		cl_device_id * /* devices */, cl_uint * /* num_devices */);

cl_int (*rclGetDeviceInfo)(cl_device_id /* device */,
		cl_device_info /* param_name */, size_t /* param_value_size */,
		void * /* param_value */, size_t * /* param_value_size_ret */);

void initFunc() {
	loadedCL = 0;
	void *handle = getCLHandle();
	if (handle == NULL)
		return;
	rclGetPlatformIDs = (cl_int (*)(cl_uint, cl_platform_id *, cl_uint*)) dlsym(
			handle, "clGetPlatformIDs");
	rclGetPlatformInfo = (cl_int (*)(cl_platform_id, cl_platform_info, size_t,
			void *, size_t*)) dlsym(handle, "clGetPlatformInfo");
	rclGetDeviceIDs = (cl_int (*)(cl_platform_id, cl_device_type, cl_uint,
			cl_device_id *, cl_uint*)) dlsym(handle, "clGetDeviceIDs");
	rclGetDeviceInfo = (cl_int (*)(cl_device_id, cl_device_info, size_t, void *,
			size_t*)) dlsym(handle, "clGetDeviceInfo");
	loadedCL = 1;
}

const char *getClInfoString() {
	initFunc();
	if (loadedCL == 0) {
		const char *ptr = "Did not find OpenCL\n"; //情況1 找不到opencl
		return ptr;
	}
	cl_platform_id platforms[10];
	cl_uint numPlats;
	rclGetPlatformIDs(10, platforms, &numPlats);
	int i;
	char *result = (char*) malloc(64000);
	if (result == 0) {
		const char *ptr = "Can't allocate JNI memory\n"; //情況2 沒法分配jni内存
		return ptr;
	}
	result[0] = '\0';
	int index = 0;
	for (i = 0; i < numPlats; i++) { //
		char platname[100];
		rclGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platname),
				platname, NULL);
		int count = sprintf(&result[index], "%ssplit", platname); //platform
		index += count;

		char platversion[100];
		rclGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION,
				sizeof(platversion), platversion, NULL);
		count = sprintf(&result[index], "%s\n", platversion);
		index += count;

		cl_device_id devices[10];
		cl_uint ndevices;
		rclGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 10, devices,
				&ndevices);
		int j;
		int err;
		for (j = 0; j < ndevices; j++) {
			char devname[100];
			cl_device_type dev_type;
			err = rclGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(devname),
					devname, NULL);
			count = sprintf(&result[index], "%d:%s\n", j, devname);
			index += count;
			/*
			 err = rclGetDeviceInfo(devices[j], CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, NULL);
			 if (err != CL_SUCCESS){
			 index += sprintf(&result[index],"   Device Type: Unknown err %d\n",err);
			 }else if (dev_type & CL_DEVICE_TYPE_ACCELERATOR){
			 index += sprintf(&result[index],"   Device Type: CL_DEVICE_TYPE_ACCELERATOR\n");
			 }else if (dev_type & CL_DEVICE_TYPE_CPU){
			 index += printf(&result[index],"   Device Type: CL_DEVICE_TYPE_CPU\n");
			 }else if (dev_type & CL_DEVICE_TYPE_GPU){
			 index += printf("   Device Type: CL_DEVICE_TYPE_GPU\n");
			 }else if (&result[index],dev_type & CL_DEVICE_TYPE_DEFAULT){
			 index += printf(&result[index],"   Device Type: CL_DEVICE_TYPE_DEFAULT\n");
			 }
			 */
			GET_UINT(CL_DEVICE_MAX_COMPUTE_UNITS, "Max Compute Units: %u\n")
			GET_SIZET(CL_DEVICE_MAX_WORK_GROUP_SIZE,
					"Max work group size: %u\n")
			GET_UINT(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
					"Max Work Item Dimensions: %u\n")

			{
				cl_uint dims;
				err = rclGetDeviceInfo(devices[j],
				CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(dims), &dims, NULL);
				if (err != CL_SUCCESS) {
					index += sprintf(&result[index],
							"Max work item dimensions ERR: %d\n", err);
				} else {
					//checkErr(err, "clGetDeviceInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)");
					//printf("  Max work item dimensions:\t\t\t %d\n", dims);

					size_t sizes[dims];
					err = rclGetDeviceInfo(devices[j],
					CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * dims, sizes,
							NULL);
					//checkErr(err, "clGetDeviceInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES)");
					if (err != CL_SUCCESS) {
						index += sprintf(&result[index],
								"Max work item dimensions ERR: %d\n", err);
					} else {
						index += sprintf(&result[index],
								"Max work item dimensions: %d\n", dims);
						{
							unsigned int k;
							index += sprintf(&result[index],
									"Max work items: (");
							for (k = 0; k < dims; k++) {
								index += sprintf(&result[index], "%u",
										(unsigned int) sizes[k]);
								if (k != dims - 1)
									index += sprintf(&result[index], ",");
							}
							index += sprintf(&result[index], ")\n");
						}
					}
				}
			}
			GET_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
					"Preferred vector width char: %u\n")
			GET_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
					"Preferred vector width short: %u\n")
			GET_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
					"Preferred vector width int: %u\n")
			GET_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
					"Preferred vector width long: %u\n")
			GET_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
					"Preferred vector width float: %u\n")
			GET_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
					"Preferred vector width double: %u\n")
			GET_UINT(CL_DEVICE_MAX_CLOCK_FREQUENCY,
					"Max clock frequency: %uMHz\n")
			GET_UINT(CL_DEVICE_ADDRESS_BITS, "Address bits: %ubits\n")
			GET_ULONG(CL_DEVICE_MAX_MEM_ALLOC_SIZE,
					"Max memory allocation: %lu bytes\n")

			GET_BOOL(CL_DEVICE_IMAGE_SUPPORT, "Image support: %s\n")

			GET_SIZET(CL_DEVICE_MAX_PARAMETER_SIZE,
					"Max size of kernel argument: %u\n")
			GET_UINT(CL_DEVICE_MEM_BASE_ADDR_ALIGN,
					"Alignment of base addres: %u bits\n")
			GET_UINT(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
					"Minimum alignment for any datatype: %u bytes\n")

			//printf("  Single precision floating point capability\n");
			GET_BITSET_AND(cl_device_fp_config, CL_DEVICE_SINGLE_FP_CONFIG,
					CL_FP_DENORM, "Denorms: %s\n")
			GET_BITSET_AND(cl_device_fp_config, CL_DEVICE_SINGLE_FP_CONFIG,
					CL_FP_INF_NAN, "Quiet NaNs: %s\n")
			GET_BITSET_AND(cl_device_fp_config, CL_DEVICE_SINGLE_FP_CONFIG,
					CL_FP_ROUND_TO_NEAREST, "Round to nearest even: %s\n")
			GET_BITSET_AND(cl_device_fp_config, CL_DEVICE_SINGLE_FP_CONFIG,
					CL_FP_ROUND_TO_ZERO, "Round to zero: %s\n")
			GET_BITSET_AND(cl_device_fp_config, CL_DEVICE_SINGLE_FP_CONFIG,
					CL_FP_ROUND_TO_INF, "Round to +ve and infinity: %s\n")
			GET_BITSET_AND(cl_device_fp_config, CL_DEVICE_SINGLE_FP_CONFIG,
					CL_FP_FMA, "IEEE754-2008 fused multiply-add: %s\n")

			{
				cl_device_mem_cache_type cache;
				err = rclGetDeviceInfo(devices[j],
				CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cache), &cache, NULL);
				//checkErr(err, "clGetDeviceInfo(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE)");
				//printf("  Cache type:\t\t\t\t\t ");
				if (err = CL_SUCCESS) {
					switch (cache) {
					case CL_NONE:
						index += sprintf(&result[index], "Cache type:None\n");
						break;
					case CL_READ_ONLY_CACHE:
						index += sprintf(&result[index],
								"Cache type: Read only\n");
						break;
					case CL_READ_WRITE_CACHE:
						index += sprintf(&result[index],
								"Cache type:Read/Write\n");
						break;
					}
				} else {
					index += sprintf(&result[index], "Cache type err:%d\n",
							err);
				}
			}

			GET_UINT(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
					"Cache line size: %u bytes\n")
			GET_ULONG(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
					"Cache size: %lu bytes\n")
			GET_ULONG(CL_DEVICE_GLOBAL_MEM_SIZE,
					"Global memory size: %lu bytes\n")
			GET_ULONG(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
					"Constant buffer size: %lu bytes\n")
			GET_UINT(CL_DEVICE_MAX_CONSTANT_ARGS,
					"Max number of constant args: %u\n")

			{
				cl_device_local_mem_type cache;
				err = rclGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_TYPE,
						sizeof(cache), &cache, NULL);
				//checkErr(err, "clGetDeviceInfo(CL_DEVICE_LOCAL_MEM_TYPE)");
				//printf("  Local memory type:\t\t\t\t ");
				if (err = CL_SUCCESS) {
					switch (cache) {
					case CL_LOCAL:
						index += sprintf(&result[index],
								"Local memory type:Local\n");
						break;
					case CL_GLOBAL:
						index += sprintf(&result[index],
								"Local memory type:Global\n");
						break;
					}
				} else {
					index += sprintf(&result[index],
							"Local memory type err:%d\n", err);
				}

			}

			GET_ULONG(CL_DEVICE_LOCAL_MEM_SIZE,
					"Local memory size: %lu bytes\n")
			GET_SIZET(CL_DEVICE_PROFILING_TIMER_RESOLUTION,
					"Profiling timer resolution: %u\n")
			GET_BOOL_CUSTOM(CL_DEVICE_ENDIAN_LITTLE, "Device endianess: %s\n",
					"Little", "Big")
			GET_BOOL(CL_DEVICE_AVAILABLE, "Available: %s\n")
			GET_BOOL(CL_DEVICE_COMPILER_AVAILABLE, "Compiler available: %s\n")

			index += sprintf(&result[index], "Execution capabilities: \n");
			GET_BITSET_AND(cl_device_exec_capabilities,
					CL_DEVICE_EXECUTION_CAPABILITIES, CL_EXEC_KERNEL,
					"Execute OpenCL kernels: %s\n")
			GET_BITSET_AND(cl_device_exec_capabilities,
					CL_DEVICE_EXECUTION_CAPABILITIES, CL_EXEC_NATIVE_KERNEL,
					"Execute native kernels: %s\n")

			index += sprintf(&result[index], "Queue properties:\n");
			GET_BITSET_AND(cl_command_queue_properties,
					CL_DEVICE_QUEUE_PROPERTIES,
					CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
					"Out-of-Order: %s\n")
			GET_BITSET_AND(cl_command_queue_properties,
					CL_DEVICE_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE,
					"Profiling: %s\n")

			GET_STRING(CL_DEVICE_NAME, "Name: %s\n", 256);
			GET_STRING(CL_DEVICE_VENDOR, "Vendor: %s\n", 256);
			GET_STRING(CL_DRIVER_VERSION, "Driver version: %s\n", 10);
			GET_STRING(CL_DEVICE_PROFILE, "Profile: %s\n", 30);
			GET_STRING(CL_DEVICE_VERSION, "Version: %s\n", 50);
			GET_STRING(CL_DEVICE_EXTENSIONS, "Extensions: %s\n", 4096);

			index += sprintf(&result[index], "\n");
		}
	}
	return result;
}
