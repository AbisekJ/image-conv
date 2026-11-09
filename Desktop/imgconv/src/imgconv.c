#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rgb888_to_rgb565(const unsigned char *in, unsigned char *out,
                      int width, int height)
{
    int total = width * height;
    int i;

    for (i = 0; i < total; i++) {
        unsigned char r = in[i * 3];
        unsigned char g = in[i * 3 + 1];
        unsigned char b = in[i * 3 + 2];

        unsigned short pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        out[i * 2]     = pixel & 0xFF;  /* low byte  */
        out[i * 2 + 1] = pixel >> 8;    /* high byte */
    }
}

void rgb888_to_rgb555(const unsigned char *in, unsigned char *out,
                      int width, int height)
{
    int total = width * height;
    int i;

    for (i = 0; i < total; i++) {
        unsigned char r = in[i * 3];
        unsigned char g = in[i * 3 + 1];
        unsigned char b = in[i * 3 + 2];

        unsigned short pixel = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);

        out[i * 2]     = pixel & 0xFF;
        out[i * 2 + 1] = pixel >> 8;
    }
}

unsigned char clamp(float value)
{
    if (value < 0.0f) {
        return 0;
    }
    if (value > 255.0f) {
        return 255;
    }
    return (unsigned char)(value + 0.5f);
}

void rgb888_to_yuv444(const unsigned char *in, unsigned char *out,
                      int width, int height)
{
    int total = width * height;
    int i;

    for (i = 0; i < total; i++) {
        float r = in[i * 3];
        float g = in[i * 3 + 1];
        float b = in[i * 3 + 2];

        out[i * 3]     = clamp( 0.299f * r + 0.587f * g + 0.114f * b);
        out[i * 3 + 1] = clamp(-0.169f * r - 0.331f * g + 0.500f * b + 128.0f);
        out[i * 3 + 2] = clamp( 0.500f * r - 0.419f * g - 0.081f * b + 128.0f);
    }
}

void yuv444_to_rgb888(const unsigned char *in, unsigned char *out,
                      int width, int height)
{
    int total = width * height;
    int i;

    for (i = 0; i < total; i++) {
        float y = in[i * 3];
        float u = in[i * 3 + 1] - 128.0f;
        float v = in[i * 3 + 2] - 128.0f;

        out[i * 3]     = clamp(y + 1.402f * v);
        out[i * 3 + 1] = clamp(y - 0.344f * u - 0.714f * v);
        out[i * 3 + 2] = clamp(y + 1.772f * u);
    }
}

void yuv444_to_yuv422(const unsigned char *in, unsigned char *out,
                      int width, int height)
{
    int row, x;

    for (row = 0; row < height; row++) {
        for (x = 0; x < width; x += 2) {
            int p0 = (row * width + x) * 3;   
            int p1 = p0 + 3;                  
            int o  = (row * width + x) * 2;   

            out[o]     = in[p0];                                    /* Y0 */
            out[o + 1] = (in[p0 + 1] + in[p1 + 1]) / 2;             /* average U */
            out[o + 2] = in[p1];                                    /* Y1 */
            out[o + 3] = (in[p0 + 2] + in[p1 + 2]) / 2;             /* average V */
        }
    }
}

void yuv444_to_yuv420(const unsigned char *in, unsigned char *out,
                      int width, int height)
{
    int total   = width * height;
    int u_start = total;                                /* U plane begins here */
    int v_start = total + (width / 2) * (height / 2);   /* V plane begins here */
    int row, x, i;

    //Y Plane
    for (i = 0; i < total; i++) {
        out[i] = in[i * 3];
    }

    /* U and V planes: average each 2x2 block */
    for (row = 0; row < height; row += 2) {
        for (x = 0; x < width; x += 2) {
            int a = (row * width + x) * 3;   /* top-left     */
            int b = a + 3;                   /* top-right    */
            int c = a + width * 3;           /* bottom-left  */
            int d = c + 3;                   /* bottom-right */
            int pos = (row / 2) * (width / 2) + (x / 2);

            out[u_start + pos] = (in[a + 1] + in[b + 1] + in[c + 1] + in[d + 1]) / 4;
            out[v_start + pos] = (in[a + 2] + in[b + 2] + in[c + 2] + in[d + 2]) / 4;
        }
    }
}

// File operations

/* Return the size of a file in bytes, or -1 if it cannot be opened. */
long get_file_size(const char *name)
{
    FILE *fp = fopen(name, "rb");  
    long size;

    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);         
    size = ftell(fp);               
    fclose(fp);
    return size;
}

/* Read exactly 'size' bytes into buf. Return 1 on success, 0 on failure. */
int read_file(const char *name, unsigned char *buf, long size)
{
    FILE *fp = fopen(name, "rb");
    size_t count;

    if (fp == NULL) {
        return 0;
    }
    count = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    return count == (size_t)size;
}

/* Write exactly 'size' bytes from buf. Return 1 on success, 0 on failure. */
int write_file(const char *name, const unsigned char *buf, long size)
{
    FILE *fp = fopen(name, "wb");   /* "wb" = write, binary */
    size_t count;

    if (fp == NULL) {
        return 0;
    }
    count = fwrite(buf, 1, (size_t)size, fp);
    fclose(fp);
    return count == (size_t)size;
}

// Modes

#define MODE_UNKNOWN            0
#define MODE_RGB888_TO_RGB565   1
#define MODE_RGB888_TO_RGB555   2
#define MODE_RGB888_TO_YUV444   3
#define MODE_YUV444_TO_RGB888   4
#define MODE_YUV444_TO_YUV422   5
#define MODE_YUV444_TO_YUV420   6

/* Turn the mode text typed by the user into a mode number. */
int get_mode(const char *name)
{
    if (strcmp(name, "rgb888-to-rgb565") == 0) return MODE_RGB888_TO_RGB565;
    if (strcmp(name, "rgb888-to-rgb555") == 0) return MODE_RGB888_TO_RGB555;
    if (strcmp(name, "rgb888-to-yuv444") == 0) return MODE_RGB888_TO_YUV444;
    if (strcmp(name, "yuv444-to-rgb888") == 0) return MODE_YUV444_TO_RGB888;
    if (strcmp(name, "yuv444-to-yuv422") == 0) return MODE_YUV444_TO_YUV422;
    if (strcmp(name, "yuv444-to-yuv420") == 0) return MODE_YUV444_TO_YUV420;
    return MODE_UNKNOWN;
}

/* How many bytes does the output file need for this mode? */
long get_output_size(int mode, int width, int height)
{
    long pixels = (long)width * height;

    if (mode == MODE_RGB888_TO_RGB565 || mode == MODE_RGB888_TO_RGB555 ||
        mode == MODE_YUV444_TO_YUV422) {
        return pixels * 2;          /* 2 bytes per pixel */
    }
    if (mode == MODE_RGB888_TO_YUV444 || mode == MODE_YUV444_TO_RGB888) {
        return pixels * 3;          /* 3 bytes per pixel */
    }
    return pixels * 3 / 2;          /* YUV420: 1.5 bytes per pixel */
}

/* Call the right conversion function for the chosen mode. */
void convert(int mode, const unsigned char *in, unsigned char *out,
             int width, int height)
{
    if (mode == MODE_RGB888_TO_RGB565) {
        rgb888_to_rgb565(in, out, width, height);
    } else if (mode == MODE_RGB888_TO_RGB555) {
        rgb888_to_rgb555(in, out, width, height);
    } else if (mode == MODE_RGB888_TO_YUV444) {
        rgb888_to_yuv444(in, out, width, height);
    } else if (mode == MODE_YUV444_TO_RGB888) {
        yuv444_to_rgb888(in, out, width, height);
    } else if (mode == MODE_YUV444_TO_YUV422) {
        yuv444_to_yuv422(in, out, width, height);
    } else if (mode == MODE_YUV444_TO_YUV420) {
        yuv444_to_yuv420(in, out, width, height);
    }
}

// input validation

#define MAX_DIM 8192    // biggest width/height we accept 

/* Print the short usage line. */
void print_usage(void)
{
    printf("Usage: imgconv MODE -w WIDTH -H HEIGHT -i INPUT -o OUTPUT\n");
}

int is_number(const char *text)
{
    int i = 0;

    if (text[0] == '\0') {
        return 0;                       /* empty string */
    }
    while (text[i] != '\0') {
        if (text[i] < '0' || text[i] > '9') {
            return 0;                   /* found a non-digit */
        }
        i++;
    }
    return i <= 5;                      /* i = how many characters */
}

/* Convert text to a valid width/height. Return -1 (and print why) if bad. */
int get_dimension(const char *text, const char *name)
{
    int value;

    if (!is_number(text)) {
        fprintf(stderr, "Error: %s must be a whole number of 1 to 5 digits, got '%s'\n",
                name, text);
        return -1;
    }
    value = atoi(text);
    if (value < 1 || value > MAX_DIM) {
        fprintf(stderr, "Error: %s must be between 1 and %d, got %d\n",
                name, MAX_DIM, value);
        return -1;
    }
    return value;
}

int main(int argc, char *argv[])
{
    int mode;
    int width = 0;
    int height = 0;
    const char *input_name = NULL;
    const char *output_name = NULL;
    long file_size, in_size, out_size;
    unsigned char *in_buf, *out_buf;
    int i;

    /* 1. a mode must be given and must be known */
    if (argc < 2) {
        fprintf(stderr, "Error: no mode given\n");
        print_usage();
        return 1;
    }
    mode = get_mode(argv[1]);
    if (mode == MODE_UNKNOWN) {
        fprintf(stderr, "Error: unknown mode '%s'\n", argv[1]);
        return 1;
    }

    /* 2. read the options; every option needs a value after it */
    i = 2;
    while (i < argc) {
        if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[i]);
                return 1;
            }
            width = get_dimension(argv[i + 1], "width");
            if (width < 0) {
                return 1;
            }
        } else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--height") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[i]);
                return 1;
            }
            height = get_dimension(argv[i + 1], "height");
            if (height < 0) {
                return 1;
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[i]);
                return 1;
            }
            input_name = argv[i + 1];
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s needs a value\n", argv[i]);
                return 1;
            }
            output_name = argv[i + 1];
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
            return 1;
        }
        i += 2;                          /* skip the option and its value */
    }

    /* 3. everything required must be present (0 / NULL means "not given") */
    if (width == 0 || height == 0 || input_name == NULL || output_name == NULL) {
        fprintf(stderr, "Error: -w, -H, -i and -o are all required\n");
        print_usage();
        return 1;
    }

    /* 4. YUV422 and YUV420 pair up pixels, so sizes must be even */
    if ((mode == MODE_YUV444_TO_YUV422 || mode == MODE_YUV444_TO_YUV420) &&
        width % 2 != 0) {
        fprintf(stderr, "Error: this mode needs an even width, got %d\n", width);
        return 1;
    }
    if (mode == MODE_YUV444_TO_YUV420 && height % 2 != 0) {
        fprintf(stderr, "Error: this mode needs an even height, got %d\n", height);
        return 1;
    }

    /* 5. do not let the output overwrite the input */
    if (strcmp(input_name, output_name) == 0) {
        fprintf(stderr, "Error: input and output must be different files\n");
        return 1;
    }

    /* 6. the input file must exist and have exactly the right size */
    in_size   = (long)width * height * 3;
    file_size = get_file_size(input_name);
    if (file_size < 0) {
        fprintf(stderr, "Error: cannot open input file '%s'\n", input_name);
        return 1;
    }
    if (file_size != in_size) {
        fprintf(stderr, "Error: input file is %ld bytes, expected %ld (%d x %d x 3)\n",
                file_size, in_size, width, height);
        return 1;
    }

    /* 7. allocate memory, convert, write */
    out_size = get_output_size(mode, width, height);
    in_buf   = malloc(in_size);
    out_buf  = malloc(out_size);
    if (in_buf == NULL || out_buf == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        free(in_buf);
        free(out_buf);
        return 1;
    }
    if (!read_file(input_name, in_buf, in_size)) {
        fprintf(stderr, "Error: failed to read '%s'\n", input_name);
        free(in_buf);
        free(out_buf);
        return 1;
    }

    convert(mode, in_buf, out_buf, width, height);

    if (!write_file(output_name, out_buf, out_size)) {
        fprintf(stderr, "Error: failed to write '%s'\n", output_name);
        free(in_buf);
        free(out_buf);
        return 1;
    }

    printf("Done: wrote %ld bytes to %s\n", out_size, output_name);
    free(in_buf);
    free(out_buf);
    return 0;
}
