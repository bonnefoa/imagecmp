#include <cl_util.h>

int round_up_power_of_two(int num)
{
        num--;
        num |= num >> 1;
        num |= num >> 2;
        num |= num >> 4;
        num |= num >> 8;
        num |= num >> 16;
        num++;
        return num;
}

char * read_file(const char * filename)
{
        int length;
        FILE *f = fopen(filename, "r");
        void *buffer;
        if (!f) {
                fprintf(stderr, "Unable to open %s for reading\n", filename);
                return NULL;
        }
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length+1);
        length = fread(buffer, 1, length, f);
        fclose(f);
        ((char*)buffer)[length] = '\0';
        return buffer;
}

void printStringInfo(clinfo_t clinfo, int typeStruct)
{
        char* value;
        size_t valueSize;
        clGetDeviceInfo(clinfo.device_id, typeStruct, 0, NULL, &valueSize);
        value = (char*) malloc(valueSize);
        clGetDeviceInfo(clinfo.device_id, typeStruct, valueSize, value, NULL);
        printf("%s\n", value);
        free(value);
}

void print_cl_profiling(cl_event event)
{
        cl_ulong start;
        cl_ulong end;
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START
                                , sizeof(cl_ulong), &start, NULL );
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END
                                , sizeof(cl_ulong), &end, NULL );
        printf("Time : %f ms\n", (end - start) * 1e-06);
}

void print_cl_info(clinfo_t clinfo)
{

        printStringInfo(clinfo, CL_DEVICE_NAME);

        size_t size;
        clGetDeviceInfo(clinfo.device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE
                        , sizeof(size), &size, NULL);
        printf("Max work group size %zu\n", size);

        size_t itemSize[3];
        clGetDeviceInfo(clinfo.device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES
                        , 3 * sizeof(size), &itemSize, NULL);
        printf("Max work item size %zu %zu %zu\n", itemSize[0]
               , itemSize[1], itemSize[2]);

        cl_bool sup;
        clGetDeviceInfo(clinfo.device_id, CL_DEVICE_IMAGE_SUPPORT
                        , sizeof(sup), &sup, NULL);
        printf("Image supported : %i\n", sup);

        size_t image_width;
        clGetDeviceInfo(clinfo.device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH
                        , sizeof(image_width), &image_width, NULL);

        size_t image_height;
        clGetDeviceInfo(clinfo.device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT
                        , sizeof(image_height), &image_height, NULL);
        printf("Image max resolution %zu / %zu\n", image_width, image_height);
}

size_t get_kernel_group(clinfo_t clinfo)
{
        size_t kernelSize;
        cl_int err = clGetKernelWorkGroupInfo(clinfo.kernel, clinfo.device_id
                                              , CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelSize), &kernelSize, NULL);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Failed to retrieve kernel group info\n");
                return 0;
        }
        return kernelSize;
}

clinfo_t clinfo_init(const char * kernelSource, const char * kernel_name)
{
        int devType = CL_DEVICE_TYPE_ALL;

        cl_int err;
        clinfo_t clinfo;

        err = clGetPlatformIDs(1, &(clinfo.cl_plateform), NULL);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Failed to find plateform\n");
                exit( EXIT_FAILURE );
        }

        err = clGetDeviceIDs(clinfo.cl_plateform, devType, 4, &(clinfo.device_id), NULL);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Failed to get devices\n");
                exit( EXIT_FAILURE );
        }

        clinfo.context = clCreateContext(0, 1, &(clinfo.device_id), NULL, NULL, &err);
        if(!clinfo.context) {
                fprintf(stderr, "Failed to create context\n");
                exit( EXIT_FAILURE );
        }

        clinfo.command_queue = clCreateCommandQueue(clinfo.context
                                , clinfo.device_id, CL_QUEUE_PROFILING_ENABLE, &err);
        if(!clinfo.command_queue) {
                fprintf(stderr, "Failed to create commands\n");
                exit( EXIT_FAILURE );
        }

        const char * kernelContent = read_file(kernelSource);
        clinfo.program = clCreateProgramWithSource(clinfo.context, 1
                           , &kernelContent, NULL, &err);
        if(!clinfo.program) {
                fprintf(stderr, "Failed to create program\n");
                exit( EXIT_FAILURE );
        }

        err = clBuildProgram(clinfo.program, 0, NULL, NULL, NULL, NULL);
        if(err != CL_SUCCESS) {
                size_t len;
                char buffer[2048];

                fprintf(stderr, "Failed to build program\n");
                clGetProgramBuildInfo(clinfo.program, clinfo.device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
                fprintf(stderr, "%s\n", buffer);
                exit( EXIT_FAILURE );
        }

        clinfo.kernel = clCreateKernel(clinfo.program, kernel_name, &err);
        if(!clinfo.kernel || err != CL_SUCCESS) {
                fprintf(stderr, "Failed to create kernel\n");
                exit( EXIT_FAILURE );
        }

        return clinfo;
}

void clinfo_free(clinfo_t clinfo)
{
        clReleaseProgram(clinfo.program);
        clReleaseKernel(clinfo.kernel);
        clReleaseCommandQueue(clinfo.command_queue);
        clReleaseContext(clinfo.context);
}

cl_mem push_image(clinfo_t clinfo, image_t * image, cl_event * event)
{
        cl_mem image_buffer;
        cl_int err;
        cl_image_format img_fmt;
        img_fmt.image_channel_order = CL_RGBA;
        img_fmt.image_channel_data_type = CL_UNSIGNED_INT8;
        image_buffer = clCreateImage2D(clinfo.context, CL_MEM_READ_ONLY
                                      , &img_fmt, (*image).size[0], (*image).size[1], 0, 0, &err);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Failed to create image buffer, %i\n", err);
                return NULL;
        }
        size_t origin[] = {0,0,0};
        size_t region[] = {(*image).size[0], (*image).size[1], 1};
        printf("Pushing image of size %i/%i\n", (*image).size[0]
                        , (*image).size[1]);
        err = clEnqueueWriteImage(clinfo.command_queue, image_buffer
                                  , CL_FALSE, origin, region
                                  , 0, 0, *(*image).pixels, 0, NULL, event);
        if(err != CL_SUCCESS) {
                fprintf(stderr, "Failed to write image to memory %i\n", err);
                return NULL;
        }
        return image_buffer;
}
