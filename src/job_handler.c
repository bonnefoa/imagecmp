#include <job_handler.h>

list_t * process_files(list_t * files, float threshold)
{
        clinfo_t clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION);
        list_t * similar_files = NULL;
        list_t * histograms = NULL;
        list_t * current = files;
        list_t * lst_files = NULL;
        while(current != NULL) {
                char * file = (*current).value;
                current = (*current).next;
                job_t * job = job_init();
                generate_histogram_from_file(file, clinfo, job);
                histogram_t * histo = malloc(sizeof(histogram_t));
                (*histo).file = file;
                (*histo).results = (*job).results;
                histograms = list_append(histograms, histo);
                free(job);
        }
        clinfo_free(clinfo);
        current = histograms;
        while(current != NULL) {
                lst_files = search_similar((*current).value
                                , (*current).next
                                , threshold);
                current = (*current).next;
                similar_files = list_concat(lst_files, similar_files);
        }
        current = similar_files;
        list_release(histograms);
        return similar_files;
}

list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold)
{
        list_t * lst_files = NULL;
        while(histograms != NULL) {
                histogram_t * histogram = (*histograms).value;
                histograms = (*histograms).next;
                float dist = histogram_distance((*reference).results
                                , (*histogram).results);
                if(dist < threshold) {
                        printf("Files %s and %s are similar (distance %.2f)\n"
                                        , (*reference).file
                                        , (*histogram).file
                                        , dist);
                        lst_files = list_append(lst_files, (*histogram).file);
                }
        }
        if(lst_files != NULL) {
                lst_files = list_append(lst_files, (*reference).file);
        }
        return lst_files;
}
