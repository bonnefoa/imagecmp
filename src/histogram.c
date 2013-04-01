#include <histogram.h>
#include <math.h>

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
