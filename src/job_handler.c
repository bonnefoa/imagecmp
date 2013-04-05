#include <job_handler.h>
#include <string.h>

histogram_t * wait_job(job_t *job)
{
        clWaitForEvents(1, job->fetch_event);
        histogram_t * histo = histogram_init();
        histo->file = malloc(strlen(job->name) + 1);
        strcpy(histo->file, job->name);
        int size = job->result_size[0] * job->result_size[1];
        histo->results = histogram_average(job->results, size);
        job_free(job);
        return histo;
}

list_t * push_jobs(list_t * files, clinfo_t * clinfo
                , list_t **histograms, FILE* cache_file)
{
        int code;
        list_t * job_waits = NULL;
        int count = 0;
        while(files != NULL) {
                char * filename = files->value;
                files = files->next;

                image_t * image = image_init();
                image->path = filename;
                image = read_image(image);
                if (image == NULL) {
                        continue;
                }
                if (image->size[0] > clinfo->max_width
                                || image->size[1] > clinfo->max_heigth) {
                        printf("Ignoring %s, width=%i, height=%i\n", filename
                                        , image->size[0], image->size[1]);
                        image_free(image);
                        continue;
                }

                job_t * job = job_init();
                printf("Processing image %s, width=%i, height=%i\n", filename
                                , image->size[0], image->size[1]);
                code = init_job_from_image(image, job);
                if(code == EXIT_FAILURE) {
                        fprintf(stderr, "Could not init job from image %i\n"
                                        , code);
                        return NULL;
                }
                generate_histogram(clinfo, image, job);
                clFlush(clinfo->command_queue);
                count++;
                if ( count > 50 ) {
                        histogram_t *histo = wait_job(job);
                        *histograms = list_append(*histograms, histo);
                        count--;
                } else {
                        job_waits = list_append(job_waits, job);
                }
        }
        return job_waits;
}

list_t* wait_for_jobs(list_t * job_waits)
{
        list_t * histograms = NULL;
        while(job_waits != NULL) {
                job_t * job = job_waits->value;
                histogram_t *histogram = wait_job(job);
                histograms = list_append(histograms, histogram);
                job_waits->value = NULL;
                job_waits = job_waits->next;
        }
        return histograms;
}

list_t * process_job_results(list_t * histograms, float threshold)
{
        list_t * similar_files = NULL;
        list_t * lst_files = NULL;
        while(histograms != NULL) {
                lst_files = search_similar(histograms->value
                                , histograms->next, threshold);
                histograms = histograms->next;
                similar_files = list_append(similar_files, lst_files);
        }
        return similar_files;
}

list_t * process_files(list_t * files, float threshold)
{
        clinfo_t * clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION);
        list_t * similar_files = NULL;
        list_t ** histograms = malloc(sizeof(list_t *));
        FILE *cache_file = fopen(".histograms", "wb");

        *histograms = NULL;
        list_t * job_waits = NULL;

        job_waits = push_jobs(files, clinfo, histograms, cache_file);

        list_t *last_histograms = wait_for_jobs(job_waits);
        *histograms = list_append(*histograms, last_histograms);
        list_release(job_waits);

        similar_files = process_job_results(*histograms, threshold);
        list_release_custom(*histograms, (void (*)(void *))&histogram_free);

        clinfo_free(clinfo);
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
