/*
 * main.c - reads the command line, checks every input, runs the conversion
 *
 * Input and output files are RAW (no header, only pixel bytes).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "file_io.h"
#include "image_convert.h"
#include "modes.h"
#include "validation.h"

/* Positions inside argv[]: argv[0] is the program, argv[1] is the mode */
#define MIN_ARGC            2   /* program name + mode              */
#define FIRST_ARG_INDEX     1
#define MODE_ARG_INDEX      1
#define FIRST_OPTION_INDEX  2
#define OPTION_STEP         2   /* an option and its value          */
#define VALUE_OFFSET        1   /* the value comes right after it   */

/*
 * Usage: imgconv MODE -w WIDTH -H HEIGHT -i INPUT -o OUTPUT
 * argc = number of words typed, argv[] = the words themselves.
 * Return value: EXIT_SUCCESS (0) = success, EXIT_FAILURE (1) = error.
 */
int main(int argc, char *argv[])
{
    int selected_mode;
    int image_width  = DIM_NOT_SET;
    int image_height = DIM_NOT_SET;
    const char *input_path  = NULL;
    const char *output_path = NULL;
    long actual_input_size, expected_input_size, output_size;
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

    /* 1. a mode must be given and must be known */
    if (argc < MIN_ARGC) {
        fprintf(stderr, "Error: no mode given\n");
        print_usage();
        return EXIT_FAILURE;
    }
    selected_mode = get_mode(argv[MODE_ARG_INDEX]);
    if (selected_mode == MODE_UNKNOWN) {
        fprintf(stderr, "Error: unknown mode '%s'\n", argv[MODE_ARG_INDEX]);
        return EXIT_FAILURE;
    }

    /* 2. read the options; every option needs a value after it */
    arg_index = FIRST_OPTION_INDEX;
    while (arg_index < argc) {
        if (strcmp(argv[arg_index], OPT_WIDTH_SHORT) == 0 ||
            strcmp(argv[arg_index], OPT_WIDTH_LONG) == 0) {
            if (arg_index + VALUE_OFFSET >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[arg_index]);
                return EXIT_FAILURE;
            }
            image_width = get_dimension(argv[arg_index + VALUE_OFFSET], "width");
            if (image_width == DIM_INVALID) {
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[arg_index], OPT_HEIGHT_SHORT) == 0 ||
                   strcmp(argv[arg_index], OPT_HEIGHT_LONG) == 0) {
            if (arg_index + VALUE_OFFSET >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[arg_index]);
                return EXIT_FAILURE;
            }
            image_height = get_dimension(argv[arg_index + VALUE_OFFSET], "height");
            if (image_height == DIM_INVALID) {
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[arg_index], OPT_INPUT_SHORT) == 0 ||
                   strcmp(argv[arg_index], OPT_INPUT_LONG) == 0) {
            if (arg_index + VALUE_OFFSET >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[arg_index]);
                return EXIT_FAILURE;
            }
            input_path = argv[arg_index + VALUE_OFFSET];
        } else if (strcmp(argv[arg_index], OPT_OUTPUT_SHORT) == 0 ||
                   strcmp(argv[arg_index], OPT_OUTPUT_LONG) == 0) {
            if (arg_index + VALUE_OFFSET >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[arg_index]);
                return EXIT_FAILURE;
            }
            output_path = argv[arg_index + VALUE_OFFSET];
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[arg_index]);
            return EXIT_FAILURE;
        }
        arg_index += OPTION_STEP;        /* skip the option and its value */
    }

    /* 3. everything required must be present (DIM_NOT_SET / NULL = "not given") */
    if (image_width == DIM_NOT_SET || image_height == DIM_NOT_SET ||
        input_path == NULL || output_path == NULL) {
        fprintf(stderr, "Error: -w, -H, -i and -o are all required\n");
        print_usage();
        return EXIT_FAILURE;
    }

    /* 4. YUV422 and YUV420 pair up pixels, so sizes must be even */
    if ((selected_mode == MODE_YUV444_TO_YUV422 ||
         selected_mode == MODE_YUV444_TO_YUV420) &&
        image_width % CHROMA_STEP != 0) {
        fprintf(stderr, "Error: this mode needs an even width, got %d\n", image_width);
        return EXIT_FAILURE;
    }
    if (selected_mode == MODE_YUV444_TO_YUV420 && image_height % CHROMA_STEP != 0) {
        fprintf(stderr, "Error: this mode needs an even height, got %d\n", image_height);
        return EXIT_FAILURE;
    }

    /* 5. do not let the output overwrite the input */
    if (strcmp(input_path, output_path) == 0) {
        fprintf(stderr, "Error: input and output must be different files\n");
        return EXIT_FAILURE;
    }

    /* 6. the input file must exist and have exactly the right size */
    expected_input_size = (long)image_width * image_height * INPUT_BYTES_PER_PIXEL;
    actual_input_size   = get_file_size(input_path);
    if (actual_input_size == FILE_SIZE_ERROR) {
        fprintf(stderr, "Error: cannot open input file '%s'\n", input_path);
        return EXIT_FAILURE;
    }
    if (actual_input_size != expected_input_size) {
        fprintf(stderr, "Error: input file is %ld bytes, expected %ld (%d x %d x %d)\n",
                actual_input_size, expected_input_size, image_width, image_height,
                INPUT_BYTES_PER_PIXEL);
        return EXIT_FAILURE;
    }

    /* 7. allocate memory, convert, write */
    output_size = get_output_size(selected_mode, image_width, image_height);
    input_buf   = malloc(expected_input_size);
    output_buf  = malloc(output_size);
    if (input_buf == NULL || output_buf == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        free(input_buf);
        free(output_buf);
        return EXIT_FAILURE;
    }
    if (!read_file(input_path, input_buf, expected_input_size)) {
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
