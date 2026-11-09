/* cli.c - text printed for --help, --version and usage */
#include <stdio.h>
#include "cli.h"
#include "validation.h"

/* Print the short usage line. */
void print_usage(void)
{
    printf("Usage: " PROGRAM_NAME " MODE -w WIDTH -H HEIGHT -i INPUT -o OUTPUT\n");
}

void print_version(void)
{
    printf(PROGRAM_NAME " version %s\n", VERSION);
}

void print_help(void)
{
    print_usage();
    printf("\nConvert a raw (headerless) image between pixel formats.\n");
    printf("The input file is always 3 bytes per pixel (rgb888 or packed yuv444).\n");
    printf("\nModes:\n");
    printf("  rgb888-to-rgb565   2 bytes per pixel output\n");
    printf("  rgb888-to-rgb555   2 bytes per pixel output\n");
    printf("  rgb888-to-yuv444   3 bytes per pixel output\n");
    printf("  yuv444-to-rgb888   3 bytes per pixel output\n");
    printf("  yuv444-to-yuv422   packed YUYV output (width must be even)\n");
    printf("  yuv444-to-yuv420   planar I420 output (width and height must be even)\n");
    printf("\nOptions:\n");
    printf("  -w, --width N      image width in pixels  (%d to %d)\n", MIN_DIM, MAX_DIM);
    printf("  -H, --height N     image height in pixels (%d to %d)\n", MIN_DIM, MAX_DIM);
    printf("  -i, --input FILE   input raw file\n");
    printf("  -o, --output FILE  output raw file\n");
    printf("  -h, --help         show this help\n");
    printf("  -v, --version      show version\n");
    printf("\nExample:\n");
    printf("  " PROGRAM_NAME " rgb888-to-rgb565 -w 640 -H 480 -i in.rgb -o out.565\n");
}
