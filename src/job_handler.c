#include <job_handler.h>
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

list_t * push_jobs(list_t * files, clinfo_t clinfo)
{
        int code;
        list_t * job_waits = NULL;
        while(files != NULL) {
                char * filename = files->value;
                files = files->next;
                job_t * job = job_init();
                image_t * image = image_init();
                printf("Processing file %s\n", filename);
                image->path = filename;
                image = read_image(image);
                printf("Processing image %s, width=%i, height=%i\n", filename
                                , image->size[0], image->size[1]);
                code = init_job_from_image(image, job);
                if(code == EXIT_FAILURE) {
                        fprintf(stderr, "Could not init job from image %i\n"
                                        , code);
                        return NULL;
                }
                generate_histogram(clinfo, image, job);
                clFlush(clinfo.command_queue);
                job_waits = list_append(job_waits, job);
        }
        return job_waits;
}

list_t * wait_job_results(list_t * job_waits)
{
        list_t * histograms = NULL;
        while(job_waits != NULL) {
                job_t * job = job_waits->value;
                clWaitForEvents(1, job->fetch_event);
                job_waits = job_waits->next;
                histogram_t * histo = histogram_init();
                histo->file = malloc(strlen(job->name) + 1);
                strcpy(histo->file, job->name);
                int size = job->result_size[0] * job->result_size[1];
                histo->results = histogram_average(job->results, size);
                histograms = list_append(histograms, histo);
                job_free(job);
                job_waits->value = NULL;
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
        clinfo_t clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION);
        list_t * similar_files = NULL;
        list_t * histograms = NULL;
        list_t * job_waits = NULL;

        job_waits = push_jobs(files, clinfo);

        histograms = wait_job_results(job_waits);
        list_release(job_waits);

        similar_files = process_job_results(histograms, threshold);
        list_release_custom(histograms, &histogram_free);

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
