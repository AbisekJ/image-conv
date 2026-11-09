#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Answers for functions that say yes or no */
#define TRUE  1
#define FALSE 0

/* Bytes used by one pixel in each format */
#define RGB888_BYTES_PER_PIXEL   3
#define YUV444_BYTES_PER_PIXEL   3
#define RGB565_BYTES_PER_PIXEL   2
#define RGB555_BYTES_PER_PIXEL   2
#define YUV422_BYTES_PER_PIXEL   2
#define INPUT_BYTES_PER_PIXEL    3   

/* YUV420 uses 1.5 bytes per pixel, written as the fraction 3 / 2 */
#define YUV420_SIZE_NUMERATOR    3
#define YUV420_SIZE_DENOMINATOR  2

/* YUV422 / YUV420 share one U and V between 2 pixels in each direction */
#define CHROMA_STEP	2

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

/* RGB to YUV full-range conversion factors */
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

/* Returned by get_file_size() when the file cannot be opened */
#define FILE_SIZE_ERROR     (-1)

#define NO_OFFSET           0   /* fseek offset: measure from SEEK_END exactly */
#define BYTE_SIZE           1   /* fread/fwrite item size: one byte at a time  */

/* Number given to each conversion */
#define MODE_UNKNOWN            0
#define MODE_RGB888_TO_RGB565   1
#define MODE_RGB888_TO_RGB555   2
#define MODE_RGB888_TO_YUV444   3
#define MODE_YUV444_TO_RGB888   4
#define MODE_YUV444_TO_YUV422   5
#define MODE_YUV444_TO_YUV420   6

/* Mode names typed on the command line */
#define MODE_NAME_RGB888_TO_RGB565   "rgb888-to-rgb565"
#define MODE_NAME_RGB888_TO_RGB555   "rgb888-to-rgb555"
#define MODE_NAME_RGB888_TO_YUV444   "rgb888-to-yuv444"
#define MODE_NAME_YUV444_TO_RGB888   "yuv444-to-rgb888"
#define MODE_NAME_YUV444_TO_YUV422   "yuv444-to-yuv422"
#define MODE_NAME_YUV444_TO_YUV420   "yuv444-to-yuv420"

#define MIN_DIM             1       
#define MAX_DIM             8192    
#define MAX_DIM_DIGITS      5       
#define DIM_NOT_SET         0       
#define DIM_INVALID         (-1)   

#define END_OF_STRING       '\0'
#define DIGIT_FIRST         '0'
#define DIGIT_LAST          '9'

#define PROGRAM_NAME        "imgconv"
#define VERSION             "1.0.0"

/* Command line option names */
#define OPT_WIDTH_SHORT     "-w"
#define OPT_WIDTH_LONG      "--width"
#define OPT_HEIGHT_SHORT    "-H"
#define OPT_HEIGHT_LONG     "--height"
#define OPT_INPUT_SHORT     "-i"
#define OPT_INPUT_LONG      "--input"
#define OPT_OUTPUT_SHORT    "-o"
#define OPT_OUTPUT_LONG     "--output"
#define OPT_HELP_SHORT      "-h"
#define OPT_HELP_LONG       "--help"
#define OPT_VERSION_SHORT   "-v"
#define OPT_VERSION_LONG    "--version"

/* Positions inside argv[]: argv[0] is the program, argv[1] is the mode */
#define MIN_ARGC            2   /* program name + mode              */
#define FIRST_ARG_INDEX     1
#define MODE_ARG_INDEX      1
#define FIRST_OPTION_INDEX  2
#define OPTION_STEP         2   /* an option and its value          */
#define VALUE_OFFSET        1   /* the value comes right after it   */


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

/* Return the size of a file in bytes, or FILE_SIZE_ERROR if it cannot be opened. */
long get_file_size(const char *path)
{
    FILE *file = fopen(path, "rb");     /* "rb" = read, binary */
    long size;

    if (file == NULL) {
        return FILE_SIZE_ERROR;
    }
    fseek(file, NO_OFFSET, SEEK_END);   /* jump to the end of the file */
    size = ftell(file);                 /* position at the end = file size */
    fclose(file);
    return size;
}

/* Read exactly 'size' bytes into buffer. Return TRUE on success, FALSE on failure. */
int read_file(const char *path, unsigned char *buffer, long size)
{
    FILE *file = fopen(path, "rb");
    size_t bytes_read;

    if (file == NULL) {
        return FALSE;
    }
    bytes_read = fread(buffer, BYTE_SIZE, (size_t)size, file);
    fclose(file);
    return bytes_read == (size_t)size;
}

/* Write exactly 'size' bytes from buffer. Return TRUE on success, FALSE on failure. */
int write_file(const char *path, const unsigned char *buffer, long size)
{
    FILE *file = fopen(path, "wb");     /* "wb" = write, binary */
    size_t bytes_written;

    if (file == NULL) {
        return FALSE;
    }
    bytes_written = fwrite(buffer, BYTE_SIZE, (size_t)size, file);
    fclose(file);
    return bytes_written == (size_t)size;
}

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

int is_number(const char *text)
{
    int char_count = 0;

    if (text[0] == END_OF_STRING) {
        return FALSE;                   /* empty string */
    }
    while (text[char_count] != END_OF_STRING) {
        if (text[char_count] < DIGIT_FIRST || text[char_count] > DIGIT_LAST) {
            return FALSE;               /* found a non-digit */
        }
        char_count++;
    }
    return char_count <= MAX_DIM_DIGITS;
}

/* Convert text to a valid width/height. Return DIM_INVALID (and print why) if bad. */
int get_dimension(const char *text, const char *name)
{
    int value;

    if (!is_number(text)) {
        fprintf(stderr, "Error: %s must be a whole number of 1 to %d digits, got '%s'\n",
                name, MAX_DIM_DIGITS, text);
        return DIM_INVALID;
    }
    value = atoi(text);
    if (value < MIN_DIM || value > MAX_DIM) {
        fprintf(stderr, "Error: %s must be between %d and %d, got %d\n",
                name, MIN_DIM, MAX_DIM, value);
        return DIM_INVALID;
    }
    return value;
}

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
