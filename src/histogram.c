#include <histogram.h>
#include <math.h>
#include <stdio.h>
#include <map.h>
#include <string.h>
#include <util.h>

static const char CACHE_FILE_ENTRY[] = "cache";
static Eet_Data_Descriptor *_histogram_descriptor;
static Eet_Data_Descriptor *_histogram_cache_descriptor;

void histogram_cache_descriptor_init(void)
{
        eina_init();
        eet_init();
        Eet_Data_Descriptor_Class eddc;
        EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, histogram_t);
        _histogram_descriptor = eet_data_descriptor_file_new(&eddc);

        EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, histogram_cache_t);
        _histogram_cache_descriptor = eet_data_descriptor_file_new(&eddc);

        EET_DATA_DESCRIPTOR_ADD_BASIC(_histogram_descriptor, histogram_t, "file", file, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(_histogram_descriptor
                        , histogram_t, "results", results, EET_T_FLOAT);

        EET_DATA_DESCRIPTOR_ADD_HASH(_histogram_cache_descriptor
                        , histogram_cache_t, "histograms"
                        , histograms, _histogram_descriptor);
}

void histogram_cache_descriptor_shutdown(void)
{
        eet_data_descriptor_free(_histogram_descriptor);
        eet_data_descriptor_free(_histogram_cache_descriptor);
        eet_shutdown();
        eina_shutdown();
}

histogram_t * histogram_init()
{
        histogram_t * histo = malloc(sizeof(histogram_t));
        histo->file = NULL;
        return histo;
}

void histogram_free(histogram_t * histo)
{
        free(histo->file);
        free(histo);
}

void histogram_average(float * histo, float *average, int size)
{
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
}

float histogram_distance(float * histo_1, float * histo_2)
{
        float dist = 0.f;
        for(unsigned int i = 0; i < 15; i++) {
                dist += fabs(histo_2[i] - histo_1[i]);
        }
        return fabs(dist);
}

histogram_cache_t * read_histogram_file(char * input_file)
{
        histogram_cache_t *histo_cache;
        Eet_File *ef = eet_open(input_file, EET_FILE_MODE_READ);
        if (!ef) {
                fprintf(stderr, "ERROR: could not open '%s' for read\n"
                                , input_file);
                return NULL;
        }
        histo_cache = eet_data_read(ef, _histogram_cache_descriptor
                        , CACHE_FILE_ENTRY);
        if (!histo_cache) {
                eet_close(ef);
                return NULL;
        }
        eet_close(ef);
        return histo_cache;
}

void write_histogram_to_file(char * output_file, histogram_cache_t *histos)
{
        Eet_File *ef;
        ef = eet_open(output_file, EET_FILE_MODE_WRITE);
        eet_data_write(ef, _histogram_cache_descriptor
                        , CACHE_FILE_ENTRY, histos, EINA_TRUE);
        eet_close(ef);
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
