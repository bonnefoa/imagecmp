#include <job_handler.h>
#include <string.h>

histogram_t * histogram_init()
{
        histogram_t * histo = malloc(sizeof(histogram_t));
        histo->file = NULL;
        histo->results = malloc(sizeof(float*));
        return histo;
}

void histogram_free(histogram_t * histo)
{
        free(histo->file);
        free(histo->results);
        free(histo);
}

list_t * process_files(list_t * files, float threshold)
{
        clinfo_t clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION);
        list_t * similar_files = NULL;
        list_t * histograms = NULL;
        list_t * current = files;
        list_t * lst_files = NULL;
        list_t * job_waits = NULL;
        while(current != NULL) {
                char * file = current->value;
                current = current->next;
                job_t * job = job_init();
                generate_histogram_from_file(file, clinfo, job);
                clFlush(clinfo.command_queue);
                job_waits = list_append(job_waits, job);
        }
        printf("All job submitted, processing results\n");

        current = job_waits;
        while(current != NULL) {
                job_t * job = current->value;
                clWaitForEvents(1, job->fetch_event);
                current = current->next;
                histogram_t * histo = histogram_init();
                histo->file = malloc(strlen(job->name));
                strcpy(histo->file, job->name);
                int size = job->result_size[0] * job->result_size[1];
                histo->results = histogram_average(job->results, size);
                histograms = list_append(histograms, histo);
                job_free(job);
        }

        clinfo_free(clinfo);
        current = histograms;
        while(current != NULL) {
                lst_files = search_similar(current->value
                                , current->next
                                , threshold);
                current = current->next;
                similar_files = list_append(similar_files, lst_files);
        }
        list_release(histograms);
        return similar_files;
}

list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold)
{
        list_t * lst_files = NULL;
        while(histograms != NULL) {
                histogram_t * histogram = histograms->value;
                histograms = histograms->next;
                float dist = histogram_distance(reference->results
                                , histogram->results);
                if(dist < threshold) {
                        printf("Files %s and %s are similar (distance %.2f)\n"
                                        , reference->file
                                        , histogram->file
                                        , dist);
                        lst_files = list_append(lst_files, histogram->file);
                }
        }
        if(lst_files != NULL) {
                lst_files = list_append(lst_files, reference->file);
        }
        return lst_files;
}
