#include <image_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <string.h>

image_t * image_init() {
  image_t * image = malloc(sizeof(image_t*));
  (*image).pixels = malloc(sizeof(unsigned char **));
  return image;
}

void image_free(image_t * image) {
  free((*(*image).pixels));
  free((*image).pixels);
  free(image);
}

void writeJpegImage(char * dest, image_t * image) {
    FILE *outfile;
    if ((outfile = fopen(dest, "wb")) == NULL) {
        fprintf(stderr, "can't open %s", dest);
        return;
    }
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width      = (*image).size[0];
    cinfo.image_height     = (*image).size[1];
    cinfo.input_components = RGB_CHANNEL;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, 1);
    jpeg_start_compress(&cinfo, 1);

    JSAMPROW row_pointer;
    int row_stride = cinfo.image_width * RGB_CHANNEL;
    unsigned char * pixels = *((*image).pixels);

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer = (JSAMPROW) &pixels[cinfo.next_scanline*row_stride];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}


image_t * readImage(image_t * image) {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr * error_mgr = malloc(sizeof(struct jpeg_error_mgr));

  FILE * infile;
  JSAMPARRAY buffer;
  int row_stride;

  if ((infile = fopen((*image).path, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", (*image).path);
    return NULL;
  }

  cinfo.err = jpeg_std_error(error_mgr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  (*image).size[0] = cinfo.output_width;
  (*image).size[1] = cinfo.output_height;

  row_stride = cinfo.output_width * RGB_CHANNEL;
  buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
  buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);

  unsigned char ** pixels = ((*image).pixels);
  *pixels = malloc(sizeof(unsigned char) * cinfo.output_width * cinfo.output_height * 4);

  long counter = 0;

  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    for(int i = 0; i < cinfo.output_width; i++) {
      int index = counter + i * RGBA_CHANNEL;
      (*pixels)[index]     = buffer[0][i * cinfo.output_components];
      (*pixels)[index + 1] = buffer[0][i * cinfo.output_components + 1];
      (*pixels)[index + 2] = buffer[0][i * cinfo.output_components + 2];
      (*pixels)[index + 3] = 0;
    }
    counter += cinfo.output_width * RGBA_CHANNEL;
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  return image;
}
