#include <histogram.h>
#include <math.h>
#include <stdio.h>
#include <map.h>
#include <string.h>
#include <util.h>
#include <unistd.h>

static const char CACHE_FILE_ENTRY[] = "cache";
static Eet_Data_Descriptor *_histogram_descriptor;
static Eet_Data_Descriptor *_histogram_cache_descriptor;
static Eet_File *cache_file = NULL;

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
                        , histogram_cache_t, "map_histo"
                        , map_histo, _histogram_descriptor);
}

void histogram_cache_descriptor_shutdown(void)
{
        eet_data_descriptor_free(_histogram_descriptor);
        eet_data_descriptor_free(_histogram_cache_descriptor);
        if(cache_file) {
                eet_close(cache_file);
        }
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

Eina_Hash *read_histogram_file(char * input_file)
{
        histogram_cache_t *histo_cache;
        Eina_Hash *map_histo;
        Eet_File *ef = eet_open(input_file, EET_FILE_MODE_READ);
        if (!ef) {
                map_histo = eina_hash_string_small_new(
                                (void (*)(void *))&histogram_free);
                return map_histo;
        }

        histo_cache = eet_data_read(ef, _histogram_cache_descriptor
                        , CACHE_FILE_ENTRY);
        if(cache_file) {
                eet_close(cache_file);
        }
        cache_file = ef;
        map_histo = histo_cache->map_histo;
        free(histo_cache);
        return map_histo;
}

void write_histogram_to_file(char * output_file, Eina_Hash *map_histo)
{
        Eet_File *ef;
        histogram_cache_t *cache_histo;

        printf("Writing cache map with %i entries\n"
                        , eina_hash_population(map_histo));

        unlink(output_file);
        cache_histo = malloc(sizeof(histogram_cache_t));
        cache_histo->map_histo = map_histo;

        ef = eet_open(output_file, EET_FILE_MODE_WRITE);
        eet_data_write(ef, _histogram_cache_descriptor
                        , CACHE_FILE_ENTRY, cache_histo, EINA_TRUE);

        eet_close(ef);
        free(cache_histo);
}

list_t * search_similar(histogram_t * reference, list_t * histograms
                , float threshold)
{
        list_t * lst_files = NULL;
        while(histograms) {
                histogram_t * histogram = histograms->value;
                histograms = histograms->next;
                float dist = histogram_distance(reference->results
                                , histogram->results);
                if(dist <= threshold) {
                        lst_files = list_append(lst_files
                                        , strdup(histogram->file));
                }
        }
        if(lst_files) {
                lst_files = list_append(lst_files, strdup(reference->file));
        }
        return lst_files;
}
