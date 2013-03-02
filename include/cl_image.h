#ifndef __CL_IMAGE_H__
#define __CL_IMAGE_H__

int processImageFile(char * imageSource, unsigned char ** results
    , int * width, int * height);
int processImage(unsigned char ** pixels
    , int imageWidth, int imageHeight
    , int * width, int * height, unsigned char ** results);

int searchNearestPower(int num);

#endif

