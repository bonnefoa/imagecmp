#include <cl_util.h>
#include <image_utils.h>
#include <cl_histogram.h>

job_t * job_init() {
  job_t * job = malloc(sizeof(job_t));
  return job;
}

void job_free(job_t * job) {
  free((*job).results);
  free(job);
}

int initJobFromImage(cl_struct clStruct, image_t * image, job_t * job) {
  cl_int err;
  for(int i = 0; i < 2; i++) {
    (*job).globalWorkSize[i] = roundUpPowerOfTwo((*image).size[i]);
    (*job).localWorkSize[i] = 32;
    (*job).resultSize[i] = (*job).globalWorkSize[i] / (*job).localWorkSize[i];
  }
  (*job).sizeResult = sizeof(float) * (*job).resultSize[0] * (*job).resultSize[1] * 16 * RGBA_CHANNEL;
  (*job).results = malloc((*job).sizeResult);

  (*job).outputBuffer = clCreateBuffer(clStruct.context
      , CL_MEM_WRITE_ONLY, (*job).sizeResult, NULL, &err);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to create buffer\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int generateHistogramFromFile(char * filename, cl_struct clStruct
    , job_t * jobHistogram) {
  int code;
  image_t * image = image_init();
  (*image).path = filename;

  image = readImage(image);
  printf("Processing image %s, width=%i, height=%i\n", filename
      , (*image).size[0], (*image).size[1]);
  code = initJobFromImage(clStruct, image, jobHistogram);
  code |= generateHistogram(clStruct, image, jobHistogram);

  return code;
}

int generateHistogram(cl_struct clStruct
    , image_t * image, job_t * job) {
  cl_int err;
  cl_event event;

  (*job).imageBuffer = pushImage(clStruct, image);
  if((*job).imageBuffer == NULL){
    return EXIT_FAILURE;
  }

  err = clSetKernelArg(clStruct.kernel, 0, sizeof(cl_mem), &(*job).imageBuffer);
  err |= clSetKernelArg(clStruct.kernel, 1, sizeof(cl_mem), &(*job).outputBuffer);
  err |= clSetKernelArg(clStruct.kernel, 2
      , sizeof(cl_ushort16) * (*job).localWorkSize[0] * (*job).localWorkSize[1]
      , NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Error on kernel arg set %i\n", err);
    return EXIT_FAILURE;
  }

  printf("Enqueing job global size %zu/%zu, localSize %zu/%zu\n"
      , (*job).globalWorkSize[0], (*job).globalWorkSize[1]
      , (*job).localWorkSize[0], (*job).localWorkSize[1]);
  err = clEnqueueNDRangeKernel(clStruct.commandQueue, clStruct.kernel, 2
      , NULL, (*job).globalWorkSize, (*job).localWorkSize
      , 0, NULL, &event);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Error on kernel enqueue %i\n", err);
    return EXIT_FAILURE;
  }

  clFinish(clStruct.commandQueue);
  printClProfiling(event);

  printf("Fetch %zu elements in results\n", (*job).sizeResult);
  err = clEnqueueReadBuffer(clStruct.commandQueue, (*job).outputBuffer
      , CL_TRUE, 0, (*job).sizeResult
      , (*job).results, 0, NULL, NULL);
  if(err) {
    fprintf(stderr, "Failed to read outputBuffer array\n");
    return EXIT_FAILURE;
  }

  clReleaseMemObject((*job).imageBuffer);
  clReleaseMemObject((*job).outputBuffer);

  return 0;
}
