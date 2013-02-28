#ifndef __IMAGE_UTILS_H__
#define __IMAGE_UTILS_H__

#define RGB_CHANNEL 3
#define RGBA_CHANNEL 4

void writeJpegImage(char * dest, unsigned char * pixels, int width, int height);
int readImage(char * filename, unsigned char ** pixels, int * width, int * height);

#endif
