/* image_convert.c - the six pixel format conversions */
#include "image_convert.h"

/* ---- private constants (only used in this file) ---- */

/* Position of each channel inside one pixel */
#define RED_OFFSET          0
#define GREEN_OFFSET        1
#define BLUE_OFFSET         2
#define Y_OFFSET            0
#define U_OFFSET            1
#define V_OFFSET            2

/* YUV422 packed order is Y0 U Y1 V */
#define YUYV_Y0_OFFSET      0
#define YUYV_U_OFFSET       1
#define YUYV_Y1_OFFSET      2
#define YUYV_V_OFFSET       3

/* A 16-bit pixel is stored low byte first (little-endian) */
#define LOW_BYTE_OFFSET     0
#define HIGH_BYTE_OFFSET    1
#define BYTE_MASK           0xFF
#define BITS_PER_BYTE       8

/* RGB565: bits dropped from each 8-bit channel and where each one is placed */
#define RGB565_RED_DROP     3
#define RGB565_GREEN_DROP   2
#define RGB565_BLUE_DROP    3
#define RGB565_RED_POS      11
#define RGB565_GREEN_POS    5

/* RGB555: 5 bits per channel */
#define RGB555_RED_DROP     3
#define RGB555_GREEN_DROP   3
#define RGB555_BLUE_DROP    3
#define RGB555_RED_POS      10
#define RGB555_GREEN_POS    5

/* Allowed range of one channel and rounding used by clamp() */
#define CHANNEL_MIN         0
#define CHANNEL_MAX         255
#define CHANNEL_MIN_F       0.0f
#define CHANNEL_MAX_F       255.0f
#define ROUNDING_OFFSET     0.5f

/* BT.601 full-range conversion factors */
#define Y_FROM_RED          0.299f
#define Y_FROM_GREEN        0.587f
#define Y_FROM_BLUE         0.114f
#define U_FROM_RED          (-0.169f)
#define U_FROM_GREEN        (-0.331f)
#define U_FROM_BLUE         0.500f
#define V_FROM_RED          0.500f
#define V_FROM_GREEN        (-0.419f)
#define V_FROM_BLUE         (-0.081f)
#define CHROMA_OFFSET       128.0f
#define RED_FROM_V          1.402f
#define GREEN_FROM_U        (-0.344f)
#define GREEN_FROM_V        (-0.714f)
#define BLUE_FROM_U         1.772f

/* How many pixels are averaged for one U/V value */
#define PIXELS_IN_PAIR      2
#define PIXELS_IN_BLOCK     4

/*
 * Keep a float value inside 0..255 and round it to the nearest whole number.
 * Needed because the YUV/RGB formulas can go slightly outside the range.
 */
static unsigned char clamp(float value)
{
    if (value < CHANNEL_MIN_F) {
        return CHANNEL_MIN;
    }
    if (value > CHANNEL_MAX_F) {
        return CHANNEL_MAX;
    }
    return (unsigned char)(value + ROUNDING_OFFSET);
}

/*
 * 1. RGB888 -> RGB565
 * Output bit layout: RRRRRGGG GGGBBBBB (5-6-5 bits), low byte stored first.
 * Right shift (>>) drops the low bits, left shift (<<) moves a channel
 * to its place.
 */
void rgb888_to_rgb565(const unsigned char *input, unsigned char *output,
                      int width, int height)
{
    int pixel_count = width * height;
    int pixel_index;

    for (pixel_index = 0; pixel_index < pixel_count; pixel_index++) {
        int in_pos  = pixel_index * RGB888_BYTES_PER_PIXEL;
        int out_pos = pixel_index * RGB565_BYTES_PER_PIXEL;

        unsigned char red   = input[in_pos + RED_OFFSET];
        unsigned char green = input[in_pos + GREEN_OFFSET];
        unsigned char blue  = input[in_pos + BLUE_OFFSET];

        unsigned short pixel = ((red >> RGB565_RED_DROP) << RGB565_RED_POS) |
                               ((green >> RGB565_GREEN_DROP) << RGB565_GREEN_POS) |
                               (blue >> RGB565_BLUE_DROP);

        output[out_pos + LOW_BYTE_OFFSET]  = pixel & BYTE_MASK;
        output[out_pos + HIGH_BYTE_OFFSET] = pixel >> BITS_PER_BYTE;
    }
}

/*
 * 2. RGB888 -> RGB555
 * Output bit layout: xRRRRRGG GGGBBBBB (top bit unused), low byte first.
 */
void rgb888_to_rgb555(const unsigned char *input, unsigned char *output,
                      int width, int height)
{
    int pixel_count = width * height;
    int pixel_index;

    for (pixel_index = 0; pixel_index < pixel_count; pixel_index++) {
        int in_pos  = pixel_index * RGB888_BYTES_PER_PIXEL;
        int out_pos = pixel_index * RGB555_BYTES_PER_PIXEL;

        unsigned char red   = input[in_pos + RED_OFFSET];
        unsigned char green = input[in_pos + GREEN_OFFSET];
        unsigned char blue  = input[in_pos + BLUE_OFFSET];

        unsigned short pixel = ((red >> RGB555_RED_DROP) << RGB555_RED_POS) |
                               ((green >> RGB555_GREEN_DROP) << RGB555_GREEN_POS) |
                               (blue >> RGB555_BLUE_DROP);

        output[out_pos + LOW_BYTE_OFFSET]  = pixel & BYTE_MASK;
        output[out_pos + HIGH_BYTE_OFFSET] = pixel >> BITS_PER_BYTE;
    }
}

/*
 * 3. RGB888 -> YUV444 (BT.601, full range)
 * Every pixel keeps its own Y, U and V.
 */
void rgb888_to_yuv444(const unsigned char *input, unsigned char *output,
                      int width, int height)
{
    int pixel_count = width * height;
    int pixel_index;

    for (pixel_index = 0; pixel_index < pixel_count; pixel_index++) {
        int in_pos  = pixel_index * RGB888_BYTES_PER_PIXEL;
        int out_pos = pixel_index * YUV444_BYTES_PER_PIXEL;

        float red   = input[in_pos + RED_OFFSET];
        float green = input[in_pos + GREEN_OFFSET];
        float blue  = input[in_pos + BLUE_OFFSET];

        output[out_pos + Y_OFFSET] = clamp(Y_FROM_RED * red + Y_FROM_GREEN * green +
                                           Y_FROM_BLUE * blue);
        output[out_pos + U_OFFSET] = clamp(U_FROM_RED * red + U_FROM_GREEN * green +
                                           U_FROM_BLUE * blue + CHROMA_OFFSET);
        output[out_pos + V_OFFSET] = clamp(V_FROM_RED * red + V_FROM_GREEN * green +
                                           V_FROM_BLUE * blue + CHROMA_OFFSET);
    }
}

/*
 * 4. YUV444 -> RGB888 (reverse of the formulas above)
 */
void yuv444_to_rgb888(const unsigned char *input, unsigned char *output,
                      int width, int height)
{
    int pixel_count = width * height;
    int pixel_index;

    for (pixel_index = 0; pixel_index < pixel_count; pixel_index++) {
        int in_pos  = pixel_index * YUV444_BYTES_PER_PIXEL;
        int out_pos = pixel_index * RGB888_BYTES_PER_PIXEL;

        float luma = input[in_pos + Y_OFFSET];
        float u    = input[in_pos + U_OFFSET] - CHROMA_OFFSET;
        float v    = input[in_pos + V_OFFSET] - CHROMA_OFFSET;

        output[out_pos + RED_OFFSET]   = clamp(luma + RED_FROM_V * v);
        output[out_pos + GREEN_OFFSET] = clamp(luma + GREEN_FROM_U * u + GREEN_FROM_V * v);
        output[out_pos + BLUE_OFFSET]  = clamp(luma + BLUE_FROM_U * u);
    }
}

/*
 * 5. YUV444 -> YUV422 (packed YUYV)
 * Two pixels next to each other share ONE U and ONE V (their average).
 * Output for each pair of pixels: Y0 U Y1 V. Width must be even.
 */
void yuv444_to_yuv422(const unsigned char *input, unsigned char *output,
                      int width, int height)
{
    int row, column;

    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column += PIXELS_IN_PAIR) {
            int first_pos  = (row * width + column) * YUV444_BYTES_PER_PIXEL;
            int second_pos = first_pos + YUV444_BYTES_PER_PIXEL;
            int out_pos    = (row * width + column) * YUV422_BYTES_PER_PIXEL;

            output[out_pos + YUYV_Y0_OFFSET] = input[first_pos + Y_OFFSET];
            output[out_pos + YUYV_U_OFFSET]  = (input[first_pos + U_OFFSET] +
                                                input[second_pos + U_OFFSET]) / PIXELS_IN_PAIR;
            output[out_pos + YUYV_Y1_OFFSET] = input[second_pos + Y_OFFSET];
            output[out_pos + YUYV_V_OFFSET]  = (input[first_pos + V_OFFSET] +
                                                input[second_pos + V_OFFSET]) / PIXELS_IN_PAIR;
        }
    }
}

/*
 * 6. YUV444 -> YUV420 (planar I420)
 * Every 2x2 block of pixels shares ONE U and ONE V (average of 4 pixels).
 * Output: [ all Y ][ all U ][ all V ]. Width and height must be even.
 */
void yuv444_to_yuv420(const unsigned char *input, unsigned char *output,
                      int width, int height)
{
    int pixel_count   = width * height;
    int u_plane_start = pixel_count;
    int v_plane_start = pixel_count + (width / CHROMA_STEP) * (height / CHROMA_STEP);
    int row, column, pixel_index;

    /* Y plane: copy the Y of every pixel */
    for (pixel_index = 0; pixel_index < pixel_count; pixel_index++) {
        output[pixel_index] = input[pixel_index * YUV444_BYTES_PER_PIXEL + Y_OFFSET];
    }

    /* U and V planes: average each 2x2 block */
    for (row = 0; row < height; row += CHROMA_STEP) {
        for (column = 0; column < width; column += CHROMA_STEP) {
            int top_left     = (row * width + column) * YUV444_BYTES_PER_PIXEL;
            int top_right    = top_left + YUV444_BYTES_PER_PIXEL;
            int bottom_left  = top_left + width * YUV444_BYTES_PER_PIXEL;
            int bottom_right = bottom_left + YUV444_BYTES_PER_PIXEL;
            int chroma_index = (row / CHROMA_STEP) * (width / CHROMA_STEP) +
                               (column / CHROMA_STEP);

            output[u_plane_start + chroma_index] =
                (input[top_left + U_OFFSET] + input[top_right + U_OFFSET] +
                 input[bottom_left + U_OFFSET] + input[bottom_right + U_OFFSET]) /
                PIXELS_IN_BLOCK;
            output[v_plane_start + chroma_index] =
                (input[top_left + V_OFFSET] + input[top_right + V_OFFSET] +
                 input[bottom_left + V_OFFSET] + input[bottom_right + V_OFFSET]) /
                PIXELS_IN_BLOCK;
        }
    }
}
