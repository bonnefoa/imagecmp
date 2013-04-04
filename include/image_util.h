#ifndef __IMAGE_UTILS_H__
#define __IMAGE_UTILS_H__

#define RGB_CHANNEL 3
#define RGBA_CHANNEL 4

#include <CL/cl.h>
#include <stdio.h>

typedef struct image {
        char * path;
        int size[2];
        unsigned char ** pixels;
        cl_image_format * image_fmt;
} image_t;

image_t * image_init();
void image_free(image_t * image);

void write_jpeg_image(char * dest, image_t * image);
void write_png_image(char * dest, image_t * image);
image_t * read_jpeg_image(image_t * image, FILE * infile);
image_t * read_png_image(image_t * image, FILE * infile);
image_t * read_image(image_t * image);

#endif
