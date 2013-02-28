#include <image_utils.h>
#include <cl_util.h>

int processImage(char * imageSource, unsigned char ** results
    , int * width, int * height) {
  cl_struct clStruct;

  int imageWidth;
  int imageHeight;
  unsigned char ** pixels = malloc(sizeof(unsigned char **));

  readImage(imageSource, pixels, &imageWidth, &imageHeight);
  printf("Processing image %s, width=%i, height=%i\n", imageSource
      , imageWidth, imageHeight);

  size_t localSizeX = 32;
  size_t localSizeY = 32;
  int groupNumberX = imageWidth / localSizeX;
  int groupNumberY = imageHeight / localSizeY;
  *width = groupNumberX;
  *height = groupNumberY;

  *results = malloc(sizeof(unsigned char) * groupNumberX * groupNumberY * RGB_CHANNEL);

  clStruct = initCl("src/kernel_image.cl", "readImage");
  printClInfos(clStruct);
  cl_int err;

  cl_image_format img_fmt;
  img_fmt.image_channel_order = CL_RGBA;
  img_fmt.image_channel_data_type = CL_UNSIGNED_INT8;

  cl_mem imageBuffer;
  cl_mem output;

  imageBuffer = clCreateImage2D(clStruct.context, CL_MEM_READ_ONLY
      , &img_fmt, imageWidth, imageHeight, 0, 0, &err);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to create image buffer, %i\n", err);
    return EXIT_FAILURE;
  }

  output = clCreateBuffer(clStruct.context, CL_MEM_WRITE_ONLY
      , sizeof(unsigned int) * groupNumberX * groupNumberY * RGB_CHANNEL, NULL, &err);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to create buffer\n");
    return EXIT_FAILURE;
  }

  cl_event event;

  size_t origin[] = {0,0,0};
  size_t region[] = {imageWidth, imageHeight, 1};
  err = clEnqueueWriteImage(clStruct.commandQueue, imageBuffer
      , CL_TRUE, origin, region, 0, 0, *pixels, 0, NULL, NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to write image to memory %i\n", err);
    return EXIT_FAILURE;
  }

  err = clSetKernelArg(clStruct.kernel, 0, sizeof(cl_mem), &imageBuffer);
  err |= clSetKernelArg(clStruct.kernel, 1, sizeof(cl_mem), &output);
  err |= clSetKernelArg(clStruct.kernel, 2
      , sizeof(cl_uint3) * localSizeX * localSizeY, NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Error on kernel arg set\n");
    return EXIT_FAILURE;
  }

  size_t kernelSize;
  err = clGetKernelWorkGroupInfo(clStruct.kernel, clStruct.deviceId
      , CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelSize), &kernelSize, NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to retrieve kernel group info\n");
    return EXIT_FAILURE;
  }
  printf("Kernel work group : %zu\n", kernelSize);

  size_t GWSize[]={imageWidth, imageHeight};
  size_t LWSize[]={localSizeX, localSizeY};
  err = clEnqueueNDRangeKernel(clStruct.commandQueue, clStruct.kernel, 2
      , NULL, GWSize, LWSize, 0, NULL, &event);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Error on kernel enqueue %i\n", err);
    return EXIT_FAILURE;
  }

  clFinish(clStruct.commandQueue);
  printClProfiling(event);

  printf("Fetch results\n");
  err = clEnqueueReadBuffer(clStruct.commandQueue, output, CL_TRUE
      , 0, groupNumberX * groupNumberY * sizeof(unsigned char) * RGB_CHANNEL
      , *results, 0, NULL, NULL);
  if(err) {
    fprintf(stderr, "Failed to read output array\n");
    return EXIT_FAILURE;
  }

  clReleaseMemObject(imageBuffer);
  clReleaseMemObject(output);
  clReleaseProgram(clStruct.program);
  clReleaseKernel(clStruct.kernel);
  clReleaseCommandQueue(clStruct.commandQueue);
  clReleaseContext(clStruct.context);

  return 0;
}
