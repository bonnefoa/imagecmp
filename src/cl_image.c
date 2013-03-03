#include <cl_image.h>
#include <image_utils.h>
#include <cl_util.h>

int searchNearestPower(int num) {
  int counter = 0;
  for( int i = num; i > 1; i >>= 1) {
    counter += 1;
  }
  return counter;
}

cl_mem pushImage(unsigned char * pixels, cl_struct clStruct
    , int imageWidth, int imageHeight){
  cl_mem imageBuffer;
  cl_int err;
  cl_image_format img_fmt;
  img_fmt.image_channel_order = CL_RGBA;
  img_fmt.image_channel_data_type = CL_UNSIGNED_INT8;

  imageBuffer = clCreateImage2D(clStruct.context, CL_MEM_READ_ONLY
      , &img_fmt, imageWidth, imageHeight, 0, 0, &err);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to create image buffer, %i\n", err);
    return NULL;
  }

  size_t origin[] = {0,0,0};
  size_t region[] = {imageWidth, imageHeight, 1};
  err = clEnqueueWriteImage(clStruct.commandQueue, imageBuffer
      , CL_TRUE, origin, region, 0, 0, pixels, 0, NULL, NULL);
  if(err != CL_SUCCESS) {
    fprintf(stderr, "Failed to write image to memory %i\n", err);
    return NULL;
  }

  return imageBuffer;
}
