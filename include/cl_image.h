#ifndef __CL_IMAGE_H__
#define __CL_IMAGE_H__

#include <cl_util.h>

int roundUpPowerOfTwo(int num);
cl_mem pushImage(unsigned char * pixels, cl_struct clStruct
    , int imageWidth, int imageHeight);

#endif

