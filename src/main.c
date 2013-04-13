#include <job_handler.h>
#include <cl_util.h>

int main(int argc, char * argv[])
{
        unsigned int plateform_id;
        unsigned int device_id;
        if(argc < 2) {
                fprintf(stderr, "You need to provide a <platform id> "
                        "<directoy>. Use make print_cl_infos "
                        "to discover your platform and device id.\n");
                exit(EXIT_FAILURE);
        }
        plateform_id = atoi(argv[1]);

        list_t *files = list_files(argv[3]);
        clinfo_t *clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION
                        , plateform_id);
        list_t *results = process_files(clinfo, files, 0.01f);

        list_t *current = results;
        while(current) {
                list_t *subcurrent = current->value;
                while(subcurrent) {
                        printf("\"%s\"\n", subcurrent->value);
                        subcurrent = subcurrent->next;
                }
                printf("\n");
                current = current->next;
        }

        return 0;
}
