#include <list.h>
#include <stdio.h>
#include <image_utils.h>
#include <cl_histogram.h>
#include <cl_util.h>

int main(int argc, char * argv[])
{
        if(argc < 1) {
                fprintf(stderr, "You need to provide a directoy "
                        "or a file as parameter\n");
                exit(EXIT_FAILURE);
        }
        char * imageSource = argv[1];

        clinfo_t clinfo = clinfo_init(KERNEL_PATH, KERNEL_FUNCTION);
        job_t * job = job_init();
        int exitCode = generate_histogram_from_file(imageSource , clinfo, job);
        clinfo_free(clinfo);

        return exitCode;
}
