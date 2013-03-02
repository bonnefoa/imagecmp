#include <list.h>
#include <cl_image.h>
#include <stdio.h>
#include <image_utils.h>

int main(int argc, char * argv[]) {
  unsigned char ** results = malloc(sizeof(unsigned char **));
  int width;
  int height;

  processImageFile(argv[1], results, &width, &height);

  writeJpegImage("/tmp/toto.jpg", *results, width, height);
  return 0;
}
