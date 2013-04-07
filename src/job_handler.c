#include <job_handler.h>
#include <string.h>
#include <histogram.h>

histogram_t * wait_job(job_t *job)
{
        clWaitForEvents(1, job->fetch_event);
        histogram_t * histo = histogram_init();
        histo->file = malloc(strlen(job->name) + 1);
        strcpy(histo->file, job->name);
        int size = job->result_size[0] * job->result_size[1];
        histogram_average(job->results, histo->results, size);
        job_free(job);
        return histo;
}

list_t * push_jobs(list_t * files, clinfo_t * clinfo
                , Eina_Hash *histograms)
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
                if (image->size[0] > *clinfo->max_width
                                || image->size[1] > *clinfo->max_heigth) {
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
                        eina_hash_add(histograms, histo->file, histo);
                        count--;
                } else {
                        job_waits = list_append(job_waits, job);
                }
        }
        return job_waits;
}

void wait_for_jobs(list_t * job_waits, Eina_Hash *map_histo)
{
        while(job_waits != NULL) {
                job_t * job = job_waits->value;
                histogram_t *histogram = wait_job(job);
                eina_hash_add(map_histo, histogram->file, histogram);
                job_waits->value = NULL;
                job_waits = job_waits->next;
        }
}

list_t * process_job_results(Eina_Hash *map_histo, float threshold)
{
        list_t * similar_files = NULL;
        list_t * lst_files = NULL;
        list_t * list_histo = NULL;
        list_t * current = NULL;

        Eina_Iterator *iter = eina_hash_iterator_data_new(map_histo);
        void **data = malloc(sizeof(void**));
        while(eina_iterator_next(iter, data)) {
                list_histo = list_append(list_histo, *data);
        }
        eina_iterator_free(iter);
        current = list_histo;

        while(current != NULL) {
                lst_files = search_similar(current->value
                                , current->next, threshold);
                current = current->next;
                similar_files = list_append(similar_files, lst_files);
        }
        list_release(list_histo);

        return similar_files;
}

list_t * process_files(list_t * files, float threshold)
{
        clinfo_t * clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION);
        list_t * similar_files = NULL;

        Eina_Hash *map_histo = eina_hash_string_small_new(
                        (void (*)(void *))&histogram_free);
        list_t * job_waits = NULL;

        job_waits = push_jobs(files, clinfo, map_histo);

        wait_for_jobs(job_waits, map_histo);
        list_release(job_waits);

        similar_files = process_job_results(map_histo, threshold);

        eina_hash_free(map_histo);
        clinfo_free(clinfo);
        return similar_files;
}
