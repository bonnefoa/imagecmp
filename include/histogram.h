#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <stdlib.h>

#define BUCKET_NUMBER 5

typedef struct histogram {
        char * file;
        float * results;
} histogram_t;

histogram_t * histogram_init();
void histogram_free(histogram_t * histogram);

float histogram_distance(float * histo_1, float * histo_2);
float * histogram_average(float * histo, int size);

#endif
