/* modes.c - the list of conversions and the code that picks one */
#include <string.h>
#include "modes.h"
#include "image_convert.h"

/* Mode names typed on the command line */
#define MODE_NAME_RGB888_TO_RGB565   "rgb888-to-rgb565"
#define MODE_NAME_RGB888_TO_RGB555   "rgb888-to-rgb555"
#define MODE_NAME_RGB888_TO_YUV444   "rgb888-to-yuv444"
#define MODE_NAME_YUV444_TO_RGB888   "yuv444-to-rgb888"
#define MODE_NAME_YUV444_TO_YUV422   "yuv444-to-yuv422"
#define MODE_NAME_YUV444_TO_YUV420   "yuv444-to-yuv420"

/* Turn the mode text typed by the user into a mode number. */
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
void convert(int mode, const unsigned char *input, unsigned char *output,
             int width, int height)
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
