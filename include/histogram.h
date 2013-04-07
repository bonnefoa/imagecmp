#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <stdlib.h>

#include <sys/time.h>
#include <util.h>
#include <Eina.h>
#include <Eet.h>

#define BUCKET_NUMBER 5

typedef struct histogram {
        char * file;
        float results[BUCKET_NUMBER];
} histogram_t;

typedef struct histogram_cache  {
        Eina_Hash *histograms;
} histogram_cache_t;

histogram_t * histogram_init();
void histogram_free(histogram_t * histogram);

float histogram_distance(float * histo_1, float * histo_2);
void histogram_average(float * histo, float *average, int size);

histogram_cache_t * read_histogram_file(char * input_file);
void write_histogram_to_file(char * output_file, histogram_cache_t *histos);
void histogram_cache_descriptor_init(void);
void histogram_cache_descriptor_shutdown(void);
list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold);

#endif
