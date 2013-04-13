#include <stdio.h>
#include <stdlib.h>
#include <cl_util.h>
#include <CL/cl.h>

void print_device(cl_device_id device)
{
        print_string_info(device, "Device name", CL_DEVICE_NAME);
        print_string_info(device, "Device version", CL_DEVICE_VERSION);
        print_string_info(device, "Driver version", CL_DRIVER_VERSION);
        print_string_info(device, "OpenCL C version", CL_DEVICE_OPENCL_C_VERSION);
        print_int_info(device, "Max compute units", CL_DEVICE_MAX_COMPUTE_UNITS);
        print_int_info(device, "Max group size", CL_DEVICE_MAX_WORK_GROUP_SIZE);
        print_int_info(device, "Max work items", CL_DEVICE_MAX_WORK_ITEM_SIZES);
        print_int_info(device, "Image width", CL_DEVICE_IMAGE2D_MAX_WIDTH);
        print_int_info(device, "Image height", CL_DEVICE_IMAGE2D_MAX_HEIGHT);
}

void print_plateform(int platform_num, cl_platform_id platform)
{
        cl_device_id* devices;
        cl_uint deviceCount;
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);
        for (unsigned int i = 0; i < deviceCount; i++) {
                printf("Platform %i, Device %i\n", platform_num + 1, i + 1);
                print_device(devices[i]);
        }
        free(devices);
}

int main()
{
        cl_uint platformCount;
        cl_platform_id* platforms;
        clGetPlatformIDs(0, NULL, &platformCount);
        printf("Get %d plateforms\n", platformCount);
        platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
        clGetPlatformIDs(platformCount, platforms, NULL);
        for (unsigned int i = 0; i < platformCount; i++) {
                print_plateform(i, platforms[i]);
        }
        free(platforms);
        return 0;
}
