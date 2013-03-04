#ifndef __CL_HISTOGRAM_H__
#define __CL_HISTOGRAM_H__

#define BUCKET_NUMBER 5

#include <cl_util.h>

int generateHistogramFromFile(cl_struct clStruct
    , char * imageSource, int * width, int * height, float ** results );
int generateHistogram(cl_struct clStruct, unsigned char ** pixels
    , int imageWidth, int imageHeight
    , int * width, int * height, float ** results);

#endif
