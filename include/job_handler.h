#ifndef JOB_HANDLER_H
#define JOB_HANDLER_H

#include <cl_histogram.h>
#include <list.h>
#include <histogram.h>

list_t * process_files(list_t * files, float threshold);
list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold);
list_t * wait_job_results(list_t * job_waits);
list_t * push_jobs(list_t * files, clinfo_t * clinfo, Eina_Hash *histograms);
list_t * process_job_results(Eina_Hash *map_histo, float threshold);

#endif
