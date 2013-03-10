#ifndef JOB_HANDLER_H
#define JOB_HANDLER_H

#include <cl_histogram.h>
#include <list.h>

typedef struct histogram {
        char * file;
        float * results;
} histogram_t;

list_t * process_files(list_t * files, float threshold);
list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold);

#endif
