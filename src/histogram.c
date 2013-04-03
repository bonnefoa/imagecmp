#include <histogram.h>
#include <math.h>
#include <stdio.h>
#include <map.h>
#include <string.h>

histogram_t * histogram_init()
{
        histogram_t * histo = malloc(sizeof(histogram_t));
        histo->file = NULL;
        histo->results = NULL;
        return histo;
}

void histogram_free(histogram_t * histo)
{
        free(histo->file);
        free(histo->results);
        free(histo);
}

float * histogram_average(float * histo, int size)
{
        float * average = malloc(sizeof(float) * BUCKET_NUMBER);
        for(int i = 0; i < BUCKET_NUMBER; i++) {
                average[i] = 0;
        }
        for(int x = 0; x < size; x++) {
                int index = x * 16;
                for(int i = 0; i < BUCKET_NUMBER; i++) {
                        average[i] += histo[index + i];
                }
        }
        for(int i = 0; i < BUCKET_NUMBER; i++) {
                average[i] /= size;
        }
        return average;
}

float histogram_distance(float * histo_1, float * histo_2)
{
        float dist = 0.f;
        for(unsigned int i = 0; i < 15; i++) {
                dist += fabs(histo_2[i] - histo_1[i]);
        }
        return fabs(dist);
}

histogram_t * read_histogram_line(FILE *input_file) {
        histogram_t *histo = histogram_init();
        return histo;
}

map_t * read_histogram_file(FILE *input_file) {
        map_t *map = map_create(10000);
        while(!feof(input_file)){
                histogram_t *histo = read_histogram_line(input_file);
                map = map_add(map, histo->file, histo);
        }
        return map;
}

void write_histogram_to_file(FILE *output_file, histogram_t *histo) {
}
