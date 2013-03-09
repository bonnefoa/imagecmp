#ifndef __CL_UTIL_H__
#define __CL_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <image_utils.h>

typedef struct cl_struct {
  cl_platform_id cpPlateform;
  cl_device_id deviceId;
  cl_context context;
  cl_command_queue commandQueue;
  cl_program program;
  cl_kernel kernel;
} cl_struct;

char * readFile(const char * filename);

cl_struct initCl(const char * kernelSource, const char * kernelName);
void cleanCl(cl_struct clStruct);
void printClInfos(cl_struct clStruct);
void printClProfiling(cl_event event);
size_t getKernelGroup(cl_struct clStruct);

int roundUpPowerOfTwo(int num);
cl_mem pushImage(cl_struct clStruct, image_t * imageInfo);

#endif
