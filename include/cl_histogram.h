#ifndef __CL_HISTOGRAM_H__
#define __CL_HISTOGRAM_H__

#define VECTOR_SIZE 16

#define KERNEL_PATH "src/kernel_image.cl"
#define KERNEL_FUNCTION "generate_histogram"
#include <cl_util.h>

typedef struct job {
        char * name;
        size_t global_size[2];
        size_t local_size[2];
        int group_number[2];
        int result_size[2];
        cl_mem * output_buffer;
        cl_mem * image_buffer;
        size_t output_size;
        float * results;
        float * fetched_results;
        image_t * image;
        cl_event * image_event;
        cl_event * enqueue_event;
        cl_event * fetch_event;

} job_t;

job_t * job_init();
void job_free(job_t * job);

int init_job_from_image(image_t * image, job_t * job);
int generate_histogram_from_file(char * filename
                              , clinfo_t * clinfo, job_t * job);
int generate_histogram(clinfo_t * clinfo
                      , image_t * image, job_t * job);
float * reduce_histogram(job_t * job);

#endif
