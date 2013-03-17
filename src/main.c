#include <job_handler.h>

int main(int argc, char * argv[])
{
        if(argc < 1) {
                fprintf(stderr, "You need to provide a directoy "
                        "or a file as parameter\n");
                exit(EXIT_FAILURE);
        }
        list_t * files = list_files(argv[1]);
        list_t * results = process_files(files, 0.05f);

        list_t * current = results;
        while(current != NULL) {
                list_t * sublist = (*current).value;
                while (sublist != NULL) {
                        printf("%s ", (char *)sublist->value);
                        sublist = sublist->next;
                }
                printf("\n");
                current = current->next;
        }
        return 0;
}
