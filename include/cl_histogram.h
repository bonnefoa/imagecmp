#ifndef __CL_HISTOGRAM_H__
#define __CL_HISTOGRAM_H__

#define BUCKET_NUMBER 5

#include <cl_util.h>

typedef struct job_t {
  size_t globalWorkSize[2];
  size_t localWorkSize[2];
  int resultSize[2];
  cl_mem outputBuffer;
  cl_mem imageBuffer;
  size_t sizeResult;
  float * results;
} job_t;

job_t * job_init();
void job_free(job_t * job);

int initJobFromImage(cl_struct clStruct, image_t * imageInfo
    , job_t * jobHistogram);
int generateHistogramFromFile(char * filename
    , cl_struct clStruct, job_t * jobHistogram);
int generateHistogram(cl_struct clStruct
    , image_t * imageInfo, job_t * jobHistogram);

#endif
