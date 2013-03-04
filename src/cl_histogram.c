#include <cl_image.h>
#include <image_utils.h>
#include <cl_histogram.h>

int generateHistogramFromFile(cl_struct clStruct, char * imageSource
    , int * resultWidth, int * resultHeight, float ** results ) {
  int imageWidth;
  int imageHeight;
  unsigned char ** pixels = malloc(sizeof(unsigned char **));

  readImage(imageSource, pixels, &imageWidth, &imageHeight);
  printf("Processing image %s, width=%i, height=%i\n", imageSource
      , imageWidth, imageHeight);
  int code = generateHistogram(clStruct, pixels, imageWidth, imageHeight
      , resultWidth, resultHeight, results);

  free(*pixels);
  free(pixels);
  return code;
}

int generateHistogram(cl_struct clStruct
    , unsigned char ** pixels
    , int imageWidth, int imageHeight
    , int * resultWidth, int * resultHeight, float ** results) {
  cl_int err;
  cl_mem outputBuffer;
  cl_event event;

  size_t localSizeX = 32;
  size_t localSizeY = 32;
  int groupNumberX = imageWidth / localSizeX;
  int groupNumberY = imageHeight / localSizeY;
  *resultWidth = groupNumberX;
  *resultHeight = groupNumberY;

  size_t numberElements = groupNumberX * groupNumberY
    * RGB_CHANNEL * BUCKET_NUMBER;
  *results = malloc(sizeof(float) * numberElements);

  cl_mem imageBuffer = pushImage(*pixels, clStruct, imageWidth, imageHeight);
  if(imageBuffer == NULL){
    return EXIT_FAILURE;
  }

  outputBuffer = clCreateBuffer(clStruct.context, CL_MEM_WRITE_ONLY
      , sizeof(float) * numberElements, NULL, &err);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to create buffer\n"); return EXIT_FAILURE;
  }

  err = clSetKernelArg(clStruct.kernel, 0, sizeof(cl_mem), &imageBuffer);
  err |= clSetKernelArg(clStruct.kernel, 1, sizeof(cl_mem), &outputBuffer);
  err |= clSetKernelArg(clStruct.kernel, 2
      , sizeof(cl_ushort16) * localSizeX * localSizeY, NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Error on kernel arg set\n");
    return EXIT_FAILURE;
  }

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
  err = clEnqueueReadBuffer(clStruct.commandQueue, outputBuffer, CL_TRUE
      , 0, sizeof(float) * numberElements
      , *results, 0, NULL, NULL);
  if(err) {
    fprintf(stderr, "Failed to read outputBuffer array\n");
    return EXIT_FAILURE;
  }

  clReleaseMemObject(imageBuffer);
  clReleaseMemObject(outputBuffer);

  return 0;
}
