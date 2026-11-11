/* cli.c - text printed for --help, --version and usage */
#include <stdio.h>
#include "cli.h"
#include "validation.h"

/* Print the short usage line. */
void print_usage(void)
{
    printf("Usage: " PROGRAM_NAME " [--help | --version]\n");
    printf("Run without arguments to start the interactive menu.\n");
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
    printf("\nOptions:\n");
    printf("  -h, --help         show this help\n");
    printf("  -v, --version      show version\n");
    printf("\nHow it works:\n");
    printf("  1. Choose a conversion from the menu by typing its number.\n");
    printf("  2. Type the image width and height (%d to %d).\n", MIN_DIM, MAX_DIM);
    printf("  3. Type the input file name (it must exist and be width x height x 3 bytes).\n");
    printf("  4. Type the output file name (you are asked before an existing file is overwritten).\n");
    printf("\nSizes: yuv444-to-yuv422 needs an even width;\n");
    printf("       yuv444-to-yuv420 needs an even width and height.\n");
}
