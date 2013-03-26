#include <cl_util.h>
#include <math.h>
#include <image_utils.h>
#include <cl_histogram.h>
#include <string.h>

job_t * job_init()
{
        job_t *job = malloc(sizeof(job_t));
        job->name = NULL;
        job->results = NULL;
        job->fetched_results = NULL;
        job->image_buffer = NULL;
        job->output_buffer = NULL;
        job->image   = NULL;
        job->image_event = malloc(sizeof(cl_event));
        job->enqueue_event = malloc(sizeof(cl_event));
        job->fetch_event = malloc(sizeof(cl_event));
        return job;
}

void job_free(job_t * job)
{
        if(job==NULL)
                return;
        free(job->name);
        free(job->fetched_results);
        free(job->results);
        free(job->image_buffer);
        free(job->output_buffer);
        free(job->image_event);
        free(job->enqueue_event);
        free(job->fetch_event);
        image_free(job->image);
        free(job);
        job = NULL;
}

int init_job_from_image(image_t * image, job_t * job)
{
        job->image = image;
        job->name = malloc(strlen(image->path) + 1);
        strcpy(job->name, image->path);
        for (int i = 0; i < 2; i++) {
                job->global_size[i] = round_up_power_of_two(image->size[i]);
                job->local_size[i] = 32;
                job->result_size[i] = image->size[i] / job->local_size[i];
                job->group_number[i] = job->global_size[i] / job->local_size[i];
        }
        job->output_size = sizeof(float) * job->group_number[0]
                * job->group_number[1] * VECTOR_SIZE;
        job->fetched_results = malloc(job->output_size);
        job->results = malloc(sizeof(float) * job->result_size[0]
                        * job->result_size[1] * VECTOR_SIZE);
        return EXIT_SUCCESS;
}

void event_callback(cl_event event, cl_int exec_status, void * args)
{
        (void)event;
        (void)exec_status;
        job_t * job = args;
        printf("Callback for file %s\n", job->name);
        job->results = reduce_histogram(job);

        printf("Releasing buffer\n");
        clReleaseMemObject(*job->image_buffer);
        clReleaseMemObject(*job->output_buffer);
        printf("Callback finished\n");
}

int generate_histogram(clinfo_t * clinfo
                      , image_t * image, job_t * job)
{
        cl_int err;

        job->output_buffer = malloc(sizeof(cl_mem));
        *job->output_buffer = clCreateBuffer(clinfo->context
                                             , CL_MEM_WRITE_ONLY, job->output_size, NULL, &err);
        if (err != CL_SUCCESS) {
                fprintf(stderr, "Failed to create buffer\n");
                return EXIT_FAILURE;
        }

        job->image_buffer = push_image(clinfo, image, job->image_event);
        if(job->image_buffer == NULL) {
                fprintf(stderr, "Could not copy image to buffer %i\n", err);
                return EXIT_FAILURE;
        }

        err = clSetKernelArg(clinfo->kernel, 0, sizeof(cl_mem), job->image_buffer);
        err |= clSetKernelArg(clinfo->kernel, 1, sizeof(cl_mem), job->output_buffer);
        err |= clSetKernelArg(clinfo->kernel, 2
                              , sizeof(cl_ushort16) * job->local_size[0] * job->local_size[1]
                              , NULL);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Error on kernel arg set %i\n", err);
                return EXIT_FAILURE;
        }

        printf("Enqueing job global size %zu/%zu, localSize %zu/%zu\n"
               , job->global_size[0], job->global_size[1]
               , job->local_size[0], job->local_size[1]);
        err = clEnqueueNDRangeKernel(clinfo->command_queue, clinfo->kernel, 2
                                     , NULL, job->global_size
                                     , job->local_size
                                     , 1, job->image_event
                                     , job->enqueue_event);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Error on kernel enqueue %i\n", err);
                return EXIT_FAILURE;
        }

        printf("Fetch %i / %i elements in results\n", job->group_number[0]
                        , job->group_number[1]);
        err = clEnqueueReadBuffer(clinfo->command_queue, *job->output_buffer
                                  , CL_FALSE, 0, job->output_size
                                  , job->fetched_results, 1
                                  , job->enqueue_event
                                  , job->fetch_event);
        if(err) {
                fprintf(stderr, "Failed to read output buffer array\n");
                return EXIT_FAILURE;
        }

        clSetEventCallback(*job->fetch_event, CL_COMPLETE
                        , &event_callback, job);
        return 0;
}


float * reduce_histogram(job_t * job)
{
        float * reduced = malloc(sizeof(float) * VECTOR_SIZE
                        * job->result_size[0] * job->result_size[1]);
        for(int y = 0; y < job->result_size[1]; y++){
                for(int x = 0; x < job->result_size[0]; x++){
                        for(int i = 0; i < VECTOR_SIZE; i++) {
                                int index = y * job->group_number[0]
                                        * VECTOR_SIZE
                                        + x * VECTOR_SIZE + i;
                                int index_reduced = y * job->result_size[0]
                                        * VECTOR_SIZE
                                        + x * VECTOR_SIZE + i;
                                reduced[index_reduced] = job->fetched_results[index];
                        }
                }
        }
        return reduced;
}

float * histogram_average(float * histo, int size)
{
        float * average = malloc(sizeof(float) * BUCKET_NUMBER);
        for(int i = 0; i < BUCKET_NUMBER; i++) {
                average[i] = 0;
        }
        for(int x = 0; x < size; x++) {
                int index = x * 16;
                for(int i = 0; i < BUCKET_NUMBER; i++) {
                        average[i] += histo[index + i];
                }
        }
        for(int i = 0; i < BUCKET_NUMBER; i++) {
                average[i] /= size;
        }
        return average;
}

float histogram_distance(float * histo_1, float * histo_2)
{
        float dist = 0.f;
        for(unsigned int i = 0; i < 15; i++) {
                dist += fabs(histo_2[i] - histo_1[i]);
        }
        return fabs(dist);
}
