#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "file_io.h"
#include "image_convert.h"
#include "input.h"
#include "modes.h"
#include "validation.h"


#define FIRST_ARG_INDEX     1
#define ARGC_WITHOUT_ARGS   1   /* argc when nothing is typed after the program name */

int main(int argc, char *argv[])
{
    int selected_mode;
    int image_width;
    int image_height;
    char input_path[MAX_LINE_LEN];
    char output_path[MAX_LINE_LEN];
    long input_size, output_size;
    unsigned char *input_buf, *output_buf;
    int arg_index;

    /* --help and --version are checked first, wherever they appear */
    for (arg_index = FIRST_ARG_INDEX; arg_index < argc; arg_index++) {
        if (strcmp(argv[arg_index], OPT_HELP_SHORT) == 0 ||
            strcmp(argv[arg_index], OPT_HELP_LONG) == 0) {
            print_help();
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[arg_index], OPT_VERSION_SHORT) == 0 ||
            strcmp(argv[arg_index], OPT_VERSION_LONG) == 0) {
            print_version();
            return EXIT_SUCCESS;
        }
    }

    /* any other command line argument is an error */
    if (argc > ARGC_WITHOUT_ARGS) {
        fprintf(stderr, "Error: unexpected argument '%s'\n", argv[FIRST_ARG_INDEX]);
        print_usage();
        return EXIT_FAILURE;
    }

    /* ask everything at run time; each answer is checked immediately */
    selected_mode = ask_mode();
    if (selected_mode == MODE_UNKNOWN) {
        return EXIT_FAILURE;
    }
    image_width = ask_dimension("width", needs_even_width(selected_mode));
    if (image_width == DIM_INVALID) {
        return EXIT_FAILURE;
    }
    image_height = ask_dimension("height", needs_even_height(selected_mode));
    if (image_height == DIM_INVALID) {
        return EXIT_FAILURE;
    }
    if (!ask_input_path(input_path, image_width, image_height, selected_mode)) {
        return EXIT_FAILURE;
    }
    if (!ask_output_path(output_path, input_path, selected_mode)) {
        return EXIT_FAILURE;
    }

    /* allocate memory, convert, write */
    input_size  = (long)image_width * image_height * INPUT_BYTES_PER_PIXEL;
    output_size = get_output_size(selected_mode, image_width, image_height);
    input_buf   = malloc(input_size);
    output_buf  = malloc(output_size);
    if (input_buf == NULL || output_buf == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        free(input_buf);
        free(output_buf);
        return EXIT_FAILURE;
    }
    if (!read_file(input_path, input_buf, input_size)) {
        fprintf(stderr, "Error: failed to read '%s'\n", input_path);
        free(input_buf);
        free(output_buf);
        return EXIT_FAILURE;
    }

    convert(selected_mode, input_buf, output_buf, image_width, image_height);

    if (!write_file(output_path, output_buf, output_size)) {
        fprintf(stderr, "Error: failed to write '%s'\n", output_path);
        free(input_buf);
        free(output_buf);
        return EXIT_FAILURE;
    }

    printf("Done: wrote %ld bytes to %s\n", output_size, output_path);
    free(input_buf);
    free(output_buf);
    return EXIT_SUCCESS;
}
