#include <cl_util.h>
#include <math.h>
#include <image_utils.h>
#include <cl_histogram.h>

job_t * job_init()
{
        job_t *job = malloc(sizeof(job_t));
        (*job).results = NULL;
        return job;
}

void job_free(job_t * job)
{
        free((*job).results);
        free(job);
}

int init_job_from_image(clinfo_t clinfo, image_t * image, job_t * job)
{
        cl_int err;
        for (int i = 0; i < 2; i++) {
                (*job).global_size[i] = round_up_power_of_two((*image).size[i]);
                (*job).local_size[i] = 32;
                (*job).result_size[i] = (*job).global_size[i] / (*job).local_size[i];
        }
        (*job).output_size = sizeof(float) * (*job).result_size[0] * (*job).result_size[1] * 16 * RGBA_CHANNEL;
        (*job).results = malloc((*job).output_size);

        (*job).output_buffer = clCreateBuffer(clinfo.context
                                             , CL_MEM_WRITE_ONLY, (*job).output_size, NULL, &err);
        if (err != CL_SUCCESS) {
                fprintf(stderr, "Failed to create buffer\n");
                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}

int generate_histogram_from_file(char * filename
                              , clinfo_t clinfo, job_t * job)
{
        int code;
        image_t * image = image_init();
        (*image).path = filename;

        image = readImage(image);
        printf("Processing image %s, width=%i, height=%i\n", filename
               , (*image).size[0], (*image).size[1]);
        code = init_job_from_image(clinfo, image, job);
        code |= generate_histogram(clinfo, image, job);

        return code;
}

int generate_histogram(clinfo_t clinfo
                      , image_t * image, job_t * job)
{
        cl_int err;
        cl_event event;

        (*job).image_buffer = push_image(clinfo, image);
        if((*job).image_buffer == NULL) {
                return EXIT_FAILURE;
        }

        err = clSetKernelArg(clinfo.kernel, 0, sizeof(cl_mem), &(*job).image_buffer);
        err |= clSetKernelArg(clinfo.kernel, 1, sizeof(cl_mem), &(*job).output_buffer);
        err |= clSetKernelArg(clinfo.kernel, 2
                              , sizeof(cl_ushort16) * (*job).local_size[0] * (*job).local_size[1]
                              , NULL);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Error on kernel arg set %i\n", err);
                return EXIT_FAILURE;
        }

        printf("Enqueing job global size %zu/%zu, localSize %zu/%zu\n"
               , (*job).global_size[0], (*job).global_size[1]
               , (*job).local_size[0], (*job).local_size[1]);
        err = clEnqueueNDRangeKernel(clinfo.command_queue, clinfo.kernel, 2
                                     , NULL, (*job).global_size
                                     , (*job).local_size
                                     , 0, NULL, &event);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Error on kernel enqueue %i\n", err);
                return EXIT_FAILURE;
        }

        clFinish(clinfo.command_queue);
        print_cl_profiling(event);

        printf("Fetch %zu elements in results\n", (*job).output_size);
        err = clEnqueueReadBuffer(clinfo.command_queue, (*job).output_buffer
                                  , CL_TRUE, 0, (*job).output_size
                                  , (*job).results, 0, NULL, NULL);
        if(err) {
                fprintf(stderr, "Failed to read output buffer array\n");
                return EXIT_FAILURE;
        }

        clReleaseMemObject((*job).image_buffer);
        clReleaseMemObject((*job).output_buffer);
        return 0;
}

float histogram_distance(float * histo_1, float * histo_2)
{
        float dist = 0.f;
        for(unsigned int i = 0; i < 15; i++) {
                dist += fabs(histo_2[i] - histo_1[i]);
        }
        return fabs(dist);
}
