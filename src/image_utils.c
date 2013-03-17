#include <image_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <string.h>
#include <png.h>
#include <zlib.h>

image_t * image_init()
{
        image_t * image = malloc(sizeof(image_t));
        image->pixels = malloc(sizeof(unsigned char *));
        *image->pixels = NULL;
        image->path = "";
        image->image_fmt = malloc(sizeof(cl_image_format));
        return image;
}

void image_free(image_t * image)
{
        if(image == NULL)
                return;
        if(image->pixels)
                free((*image->pixels));
        free(image->pixels);
        free(image->image_fmt);
        free(image);
}

unsigned char * convert_to_rgba(unsigned char * source, unsigned int width, unsigned int height)
{
        unsigned char * results = malloc(sizeof(unsigned char) * width * height * RGBA_CHANNEL);
        for (unsigned int y = 0; y < height; y++) {
                for(unsigned int x = 0; x < width; x++) {
                        unsigned int index = y * width * RGBA_CHANNEL + x * RGBA_CHANNEL;
                        unsigned int index_source = y * width * RGB_CHANNEL + x * RGB_CHANNEL;
                        results[index]     = source[index_source];
                        results[index + 1] = source[index_source + 1];
                        results[index + 2] = source[index_source + 2];
                        results[index + 3] = 0;
                }
        }
        return results;
}

image_t * read_image(image_t * image)
{
        printf("Reading file %s\n", (*image).path);
        FILE * infile;
        if ((infile = fopen(image->path, "rb")) == NULL) {
                fprintf(stderr, "can't open %s\n", (*image).path);
                return NULL;
        }
        unsigned char sig[8];
        fread(sig, 1, 8, infile);
        if (png_sig_cmp(sig, 0, 8) == 0) {
                image = read_png_image(image, infile);
        } else {
                fseek(infile, 0, 0);
                image = read_jpeg_image(image, infile);
        }
        fclose(infile);
        return image;
}

void write_png_image(char * dest, image_t * image)
{
        FILE *outfile;
        if ((outfile = fopen(dest, "wb")) == NULL) {
                fprintf(stderr, "can't open %s", dest);
                return;
        }
        png_structp  png_ptr;
        png_infop  info_ptr;

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                        NULL, NULL, NULL);
        info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, outfile);
        png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

        png_set_IHDR(png_ptr, info_ptr, (*image).size[0],
                        (*image).size[1], 8,
                        PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        for(int i = 0; i < (*image).size[1]; i++) {
                png_write_row(png_ptr
                                , &(*image->pixels)[i * (*image).size[0] * RGB_CHANNEL]);
        }
        png_write_end(png_ptr, NULL);
        fclose(outfile);
}

image_t * read_png_image(image_t * image, FILE * infile)
{
        int bit_depth;
        int color_type;
        png_uint_32  i, rowbytes;
        png_uint_32 width;
        png_uint_32 height;
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING
                        , NULL, NULL, NULL);
        png_infop info_ptr = png_create_info_struct(png_ptr);

        png_init_io(png_ptr, infile);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                        &color_type, NULL, NULL, NULL);
        (*image).size[0] = width;
        (*image).size[1] = height;
        if (color_type == PNG_COLOR_TYPE_PALETTE)
                png_set_expand(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
                png_set_expand(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
                png_set_expand(png_ptr);
        if (bit_depth == 16)
                png_set_strip_16(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY ||
                        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb(png_ptr);

        (*(*image).image_fmt).image_channel_order = CL_RGBA;
        (*(*image).image_fmt).image_channel_data_type = CL_UNSIGNED_INT8;

        png_bytep row_pointers[height];
        png_read_update_info(png_ptr, info_ptr);
        rowbytes = png_get_rowbytes(png_ptr, info_ptr);

        unsigned char * lines = malloc(sizeof(unsigned char) * width * height * RGB_CHANNEL);
        for (i = 0;  i < height;  ++i)
                row_pointers[i] = lines + i*rowbytes;
        png_read_image(png_ptr, row_pointers);
        png_read_end(png_ptr, NULL);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        *image->pixels = convert_to_rgba(lines, width, height);
        free(lines);
        return image;
}

void write_jpeg_image(char * dest, image_t * image)
{
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

image_t * read_jpeg_image(image_t * image, FILE * infile)
{
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr * error_mgr = malloc(sizeof(struct jpeg_error_mgr));
        JSAMPARRAY buffer;
        int row_stride;
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
        unsigned char * lines = malloc(sizeof(unsigned char) * cinfo.output_width * cinfo.output_height * RGB_CHANNEL);
        int counter = 0;
        while (cinfo.output_scanline < cinfo.output_height) {
                jpeg_read_scanlines(&cinfo, buffer, 1);
                for(unsigned int i = 0; i < cinfo.output_width; i++) {
                        unsigned int index = counter + i * RGB_CHANNEL;
                        lines[index]     = buffer[0][i * cinfo.output_components];
                        lines[index + 1] = buffer[0][i * cinfo.output_components + 1];
                        lines[index + 2] = buffer[0][i * cinfo.output_components + 2];
                }
                counter += cinfo.output_width * RGB_CHANNEL;
        }
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        (*image->image_fmt).image_channel_order = CL_RGBA;
        (*image->image_fmt).image_channel_data_type = CL_UNSIGNED_INT8;

        *image->pixels = convert_to_rgba(lines, cinfo.output_width, cinfo.output_height);
        free(error_mgr);
        free(buffer[0]);
        free(buffer);
        free(lines);
        return image;
}
