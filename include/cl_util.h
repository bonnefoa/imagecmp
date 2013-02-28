#ifndef __CL_UTIL_H__
#define __CL_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

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
void printClInfos(cl_struct clStruct);
void printClProfiling(cl_event event);

#endif
