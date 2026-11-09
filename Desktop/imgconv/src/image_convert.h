/* image_convert.h - the six pixel format conversions */
#ifndef IMAGE_CONVERT_H
#define IMAGE_CONVERT_H

/* Bytes used by one pixel in each format */
#define RGB888_BYTES_PER_PIXEL   3
#define YUV444_BYTES_PER_PIXEL   3
#define RGB565_BYTES_PER_PIXEL   2
#define RGB555_BYTES_PER_PIXEL   2
#define YUV422_BYTES_PER_PIXEL   2
#define INPUT_BYTES_PER_PIXEL    3   /* rgb888 and yuv444 inputs are both 3 */

/* YUV420 uses 1.5 bytes per pixel, written as the fraction 3 / 2 */
#define YUV420_SIZE_NUMERATOR    3
#define YUV420_SIZE_DENOMINATOR  2

/* YUV422 / YUV420 share one U and V between 2 pixels in each direction */
#define CHROMA_STEP              2

/*
 * Each function reads width*height pixels from input and writes the
 * converted pixels to output. The caller must provide big enough buffers.
 */
void rgb888_to_rgb565(const unsigned char *input, unsigned char *output, int width, int height);
void rgb888_to_rgb555(const unsigned char *input, unsigned char *output, int width, int height);
void rgb888_to_yuv444(const unsigned char *input, unsigned char *output, int width, int height);
void yuv444_to_rgb888(const unsigned char *input, unsigned char *output, int width, int height);
void yuv444_to_yuv422(const unsigned char *input, unsigned char *output, int width, int height);
void yuv444_to_yuv420(const unsigned char *input, unsigned char *output, int width, int height);

#endif /* IMAGE_CONVERT_H */
