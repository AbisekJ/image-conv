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

            out[o]     = in[p0];                                /* Y0 */
            out[o + 1] = (in[p0 + 1] + in[p1 + 1]) / 2;         /* average U */
            out[o + 2] = in[p1];                                /* Y1 */
            out[o + 3] = (in[p0 + 2] + in[p1 + 2]) / 2;         /* average V */
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

    // Y plane
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

//file operations 

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
    FILE *fp = fopen(name, "wb");   
    size_t count;

    if (fp == NULL) {
        return 0;
    }
    count = fwrite(buf, 1, (size_t)size, fp);
    fclose(fp);
    return count == (size_t)size;
}

#define WIDTH  4
#define HEIGHT 4

int main(void)
{
    long in_size  = (long)WIDTH * HEIGHT * 3;   /* RGB888 = 3 bytes/pixel */
    long out_size = (long)WIDTH * HEIGHT * 2;   /* RGB565 = 2 bytes/pixel */
    unsigned char *in_buf  = malloc(in_size);
    unsigned char *out_buf = malloc(out_size);

    if (in_buf == NULL || out_buf == NULL) {
        printf("Error: out of memory\n");
        free(in_buf);
        free(out_buf);
        return 1;
    }
    if (!read_file("input.raw", in_buf, in_size)) {
        printf("Error: cannot read input.raw\n");
        free(in_buf);
        free(out_buf);
        return 1;
    }

    rgb888_to_rgb565(in_buf, out_buf, WIDTH, HEIGHT);

    if (!write_file("output.raw", out_buf, out_size)) {
        printf("Error: cannot write output.raw\n");
        free(in_buf);
        free(out_buf);
        return 1;
    }

    printf("Done: wrote %ld bytes to output.raw\n", out_size);
    free(in_buf);
    free(out_buf);
    return 0;
}
