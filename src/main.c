#include <list.h>
#include <stdio.h>
#include <image_utils.h>
#include <cl_histogram.h>
#include <cl_util.h>

int main(int argc, char * argv[]) {
  char * imageSource = argv[1];

  cl_struct clStruct = initCl("src/kernel_image.cl", "generateHistogram");
  job_t * job = job_init();
  int exitCode = generateHistogramFromFile(imageSource , clStruct, job);
  cleanCl(clStruct);

  return exitCode;
}
