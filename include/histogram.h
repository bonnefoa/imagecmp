#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <stdlib.h>

#define BUCKET_NUMBER 5

float histogram_distance(float * histo_1, float * histo_2);
float * histogram_average(float * histo, int size);

#endif
