#ifndef __IMAGE_UTILS_H__
#define __IMAGE_UTILS_H__

#define RGB_CHANNEL 3
#define RGBA_CHANNEL 4

typedef struct image {
        char * path;
        int size[2];
        unsigned char ** pixels;
} image_t;

image_t * image_init();
void image_free(image_t * image);

void write_jpeg_image(char * dest, image_t * image);
image_t * readImage(image_t * image);

#endif
