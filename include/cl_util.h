#ifndef __CL_UTIL_H__
#define __CL_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <image_utils.h>

typedef struct clinfo {
        cl_platform_id cl_plateform;
        cl_device_id device_id;
        cl_context context;
        cl_command_queue command_queue;
        cl_program program;
        cl_kernel kernel;
} clinfo_t;

char * read_file(const char * filename);

clinfo_t clinfo_init(const char * kernel_source, const char * kernel_name);
void clinfo_free(clinfo_t clinfo);

void print_cl_info(clinfo_t clinfo);
void print_cl_profiling(cl_event event);
size_t get_kernel_group(clinfo_t clinfo);

int round_up_power_of_two(int num);
cl_mem push_image(clinfo_t clinfo, image_t * image, cl_event * event);

#endif
