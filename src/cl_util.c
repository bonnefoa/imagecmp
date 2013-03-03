#include <cl_util.h>

char * readFile(const char * filename) {
  int length;
  FILE *f = fopen(filename, "r");
  void *buffer;

  if (!f) {
    fprintf(stderr, "Unable to open %s for reading\n", filename);
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);

  buffer = malloc(length+1);
  length = fread(buffer, 1, length, f);
  fclose(f);
  ((char*)buffer)[length] = '\0';

  return buffer;
}

void printStringInfo(cl_struct clStruct, int typeStruct) {
  char* value;
  size_t valueSize;
  clGetDeviceInfo(clStruct.deviceId, typeStruct, 0, NULL, &valueSize);
  value = (char*) malloc(valueSize);
  clGetDeviceInfo(clStruct.deviceId, typeStruct, valueSize, value, NULL);
  printf("%s\n", value);
  free(value);
}

void printClProfiling(cl_event event){
  cl_ulong start;
  cl_ulong end;
  clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START
      , sizeof(cl_ulong), &start, NULL );
  clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END
      , sizeof(cl_ulong), &end, NULL );
  printf("Time : %f ms\n", (end - start) * 1e-06);
}

void printClInfos(cl_struct clStruct) {

  printStringInfo(clStruct, CL_DEVICE_NAME);

  size_t size;
  clGetDeviceInfo(clStruct.deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE
      , sizeof(size), &size, NULL);
  printf("Max work group size %zu\n", size);

  size_t itemSize[3];
  clGetDeviceInfo(clStruct.deviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES
      , 3 * sizeof(size), &itemSize, NULL);
  printf("Max work item size %zu %zu %zu\n", itemSize[0]
      , itemSize[1], itemSize[2]);

  cl_bool sup;
  clGetDeviceInfo(clStruct.deviceId, CL_DEVICE_IMAGE_SUPPORT
      , sizeof(sup), &sup, NULL);
  printf("Image supported : %i\n", sup);

}

size_t getKernelGroup(cl_struct clStruct) {
  size_t kernelSize;
  cl_int err = clGetKernelWorkGroupInfo(clStruct.kernel, clStruct.deviceId
      , CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelSize), &kernelSize, NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to retrieve kernel group info\n");
    return 0;
  }
  return kernelSize;
}

cl_struct initCl(const char * kernelSource, const char * kernelName) {
  int devType = CL_DEVICE_TYPE_ALL;

  cl_int err;
  cl_struct clStruct;

  err = clGetPlatformIDs(1, &(clStruct.cpPlateform), NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to find plateform\n");
    exit( EXIT_FAILURE );
  }

  err = clGetDeviceIDs(clStruct.cpPlateform, devType, 4, &(clStruct.deviceId), NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to get devices\n");
    exit( EXIT_FAILURE );
  }

  clStruct.context = clCreateContext(0, 1, &(clStruct.deviceId), NULL, NULL, &err);
  if(!clStruct.context) {
    fprintf(stderr, "Failed to create context\n");
    exit( EXIT_FAILURE );
  }

  clStruct.commandQueue = clCreateCommandQueue(clStruct.context
      , clStruct.deviceId, CL_QUEUE_PROFILING_ENABLE, &err);
  if(!clStruct.commandQueue) {
    fprintf(stderr, "Failed to create commands\n");
    exit( EXIT_FAILURE );
  }

  const char * kernelContent = readFile(kernelSource);
  clStruct.program = clCreateProgramWithSource(clStruct.context, 1
      , &kernelContent, NULL, &err);
  if(!clStruct.program) {
    fprintf(stderr, "Failed to create program\n");
    exit( EXIT_FAILURE );
  }

  err = clBuildProgram(clStruct.program, 0, NULL, NULL, NULL, NULL);
  if(err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];

    fprintf(stderr, "Failed to build program\n");
    clGetProgramBuildInfo(clStruct.program, clStruct.deviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    fprintf(stderr, "%s\n", buffer);
    exit( EXIT_FAILURE );
  }

  clStruct.kernel = clCreateKernel(clStruct.program, kernelName, &err);
  if(!clStruct.kernel || err != CL_SUCCESS) {
    fprintf(stderr, "Failed to create kernel\n");
    exit( EXIT_FAILURE );
  }

  return clStruct;
}
