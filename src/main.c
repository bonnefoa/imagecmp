#include <list.h>
#include <cl_image.h>
#include <stdio.h>
#include <image_utils.h>
#include <cl_histogram.h>
#include <cl_util.h>

int main(int argc, char * argv[]) {
  char * imageSource = argv[1];
  float ** results = malloc(sizeof(float **));
  int resultWidth;
  int resultHeight;

  cl_struct clStruct = initCl("src/kernel_image.cl", "generateHistogram");
  int exitCode = generateHistogramFromFile(clStruct, imageSource
      , &resultWidth, &resultHeight, results );
  cleanCl(clStruct);

  return exitCode;
}
