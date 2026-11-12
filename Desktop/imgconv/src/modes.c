/* modes.c - the list of conversions and the code that picks one */
#include <stdio.h>
#include "modes.h"
#include "image_convert.h"

/* Mode names typed on the command line 
#define MODE_NAME_RGB888_TO_RGB565   "rgb888-to-rgb565"
#define MODE_NAME_RGB888_TO_RGB555   "rgb888-to-rgb555"
#define MODE_NAME_RGB888_TO_YUV444   "rgb888-to-yuv444"
#define MODE_NAME_YUV444_TO_RGB888   "yuv444-to-rgb888"
#define MODE_NAME_YUV444_TO_YUV422   "yuv444-to-yuv422"
#define MODE_NAME_YUV444_TO_YUV420   "yuv444-to-yuv420" */

/* Turn the mode text typed by the user into a mode number. 
int get_mode(const char *name)
{
    if (strcmp(name, MODE_NAME_RGB888_TO_RGB565) == 0) return MODE_RGB888_TO_RGB565;
    if (strcmp(name, MODE_NAME_RGB888_TO_RGB555) == 0) return MODE_RGB888_TO_RGB555;
    if (strcmp(name, MODE_NAME_RGB888_TO_YUV444) == 0) return MODE_RGB888_TO_YUV444;
    if (strcmp(name, MODE_NAME_YUV444_TO_RGB888) == 0) return MODE_YUV444_TO_RGB888;
    if (strcmp(name, MODE_NAME_YUV444_TO_YUV422) == 0) return MODE_YUV444_TO_YUV422;
    if (strcmp(name, MODE_NAME_YUV444_TO_YUV420) == 0) return MODE_YUV444_TO_YUV420;
    return MODE_UNKNOWN;
}
*/

/* Text shown next to each number in the menu */
#define MODE_TEXT_RGB888_TO_RGB565   "RGB888 to RGB565   (2 bytes per pixel)"
#define MODE_TEXT_RGB888_TO_RGB555   "RGB888 to RGB555   (2 bytes per pixel)"
#define MODE_TEXT_RGB888_TO_YUV444   "RGB888 to YUV444   (3 bytes per pixel)"
#define MODE_TEXT_YUV444_TO_RGB888   "YUV444 to RGB888   (3 bytes per pixel)"
#define MODE_TEXT_YUV444_TO_YUV422   "YUV444 to YUV422   (packed YUYV, width must be even)"
#define MODE_TEXT_YUV444_TO_YUV420   "YUV444 to YUV420   (planar I420, width and height must be even)"

/* Print the menu: one number for each conversion. */
void print_mode_menu(void)
{
    printf("\n=============== imgconv - choose a conversion ===============\n");
    printf("  %d. %s\n", MODE_RGB888_TO_RGB565, MODE_TEXT_RGB888_TO_RGB565);
    printf("  %d. %s\n", MODE_RGB888_TO_RGB555, MODE_TEXT_RGB888_TO_RGB555);
    printf("  %d. %s\n", MODE_RGB888_TO_YUV444, MODE_TEXT_RGB888_TO_YUV444);
    printf("  %d. %s\n", MODE_YUV444_TO_RGB888, MODE_TEXT_YUV444_TO_RGB888);
    printf("  %d. %s\n", MODE_YUV444_TO_YUV422, MODE_TEXT_YUV444_TO_YUV422);
    printf("  %d. %s\n", MODE_YUV444_TO_YUV420, MODE_TEXT_YUV444_TO_YUV420);
    printf("==============================================================\n");
}

/* File extensions: .rgb for RGB data, .yuv for YUV data */
#define EXTENSION_RGB   ".rgb"
#define EXTENSION_YUV   ".yuv"

/* Extension the INPUT file must have: the three RGB888 modes read RGB data, the rest read YUV. */
const char *get_input_extension(int mode)
{
    if (mode == MODE_RGB888_TO_RGB565 || mode == MODE_RGB888_TO_RGB555 ||
        mode == MODE_RGB888_TO_YUV444) {
        return EXTENSION_RGB;
    }
    return EXTENSION_YUV;
}

/* Extension the OUTPUT file must have: RGB565, RGB555 and RGB888 are RGB data, the rest YUV. */
const char *get_output_extension(int mode)
{
    if (mode == MODE_RGB888_TO_RGB565 || mode == MODE_RGB888_TO_RGB555 ||
        mode == MODE_YUV444_TO_RGB888) {
        return EXTENSION_RGB;
    }
    return EXTENSION_YUV;
}

/* YUV422 and YUV420 pair up pixels, so they need an even width. */
int needs_even_width(int mode)
{
    return mode == MODE_YUV444_TO_YUV422 || mode == MODE_YUV444_TO_YUV420;
}

/* Only YUV420 (2x2 blocks) needs an even height. */
int needs_even_height(int mode)
{
    return mode == MODE_YUV444_TO_YUV420;
}

/* How many bytes does the output file need for this mode? */
long get_output_size(int mode, int width, int height)
{
    long pixels = (long)width * height;

    if (mode == MODE_RGB888_TO_RGB565) {
        return pixels * RGB565_BYTES_PER_PIXEL;
    }
    if (mode == MODE_RGB888_TO_RGB555) {
        return pixels * RGB555_BYTES_PER_PIXEL;
    }
    if (mode == MODE_YUV444_TO_YUV422) {
        return pixels * YUV422_BYTES_PER_PIXEL;
    }
    if (mode == MODE_RGB888_TO_YUV444) {
        return pixels * YUV444_BYTES_PER_PIXEL;
    }
    if (mode == MODE_YUV444_TO_RGB888) {
        return pixels * RGB888_BYTES_PER_PIXEL;
    }
    /* the only mode left is YUV420 */
    return pixels * YUV420_SIZE_NUMERATOR / YUV420_SIZE_DENOMINATOR;
}

/* Call the right conversion function for the chosen mode. */
void convert(int mode, const unsigned char *input, unsigned char *output, int width, int height)
{
    if (mode == MODE_RGB888_TO_RGB565) {
        rgb888_to_rgb565(input, output, width, height);
    } else if (mode == MODE_RGB888_TO_RGB555) {
        rgb888_to_rgb555(input, output, width, height);
    } else if (mode == MODE_RGB888_TO_YUV444) {
        rgb888_to_yuv444(input, output, width, height);
    } else if (mode == MODE_YUV444_TO_RGB888) {
        yuv444_to_rgb888(input, output, width, height);
    } else if (mode == MODE_YUV444_TO_YUV422) {
        yuv444_to_yuv422(input, output, width, height);
    } else if (mode == MODE_YUV444_TO_YUV420) {
        yuv444_to_yuv420(input, output, width, height);
    }
}
