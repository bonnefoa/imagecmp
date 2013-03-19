#ifndef JOB_HANDLER_H
#define JOB_HANDLER_H

#include <cl_histogram.h>
#include <list.h>

typedef struct histogram {
        char * file;
        float * results;
} histogram_t;

histogram_t * histogram_init();
void histogram_free(histogram_t * histogram);

list_t * process_files(list_t * files, float threshold);
list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold);
list_t * push_jobs(list_t * files, clinfo_t clinfo);
list_t * wait_job_results(list_t * job_waits);
list_t * process_job_results(list_t * histograms, float threshold);

#endif
