#ifndef __CL_HISTOGRAM_H__
#define __CL_HISTOGRAM_H__

#define BUCKET_NUMBER 5

#include <cl_util.h>

typedef struct job {
        size_t global_size[2];
        size_t local_size[2];
        int result_size[2];
        cl_mem output_buffer;
        cl_mem image_buffer;
        size_t output_size;
        float * results;
} job_t;

job_t * job_init();
void job_free(job_t * job);

int init_job_from_image(clinfo_t clinfo, image_t * image
                     , job_t * job);
int generate_histogram_from_file(char * filename
                              , clinfo_t clinfo, job_t * job);
int generate_histogram(clinfo_t clinfo
                      , image_t * image, job_t * job);

#endif
