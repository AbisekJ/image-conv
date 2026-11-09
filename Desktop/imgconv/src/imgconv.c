#include <stdio.h>

/* Print bytes as hex so we can see the result on screen */
void print_hex(const unsigned char *data, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

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

//For proper rounding - off
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

int main(void)
{
    /* A tiny 2x2 test image: red, green, blue, white pixels (RGB888) */
    unsigned char rgb[12] = { 255, 0, 0,   0, 255, 0,   0, 0, 255,   255, 255, 255 };

    unsigned char out565[8];
    rgb888_to_rgb565(rgb, out565, 2, 2);
    printf("RGB565 : ");
    print_hex(out565, 8);

    unsigned char out555[8];
    rgb888_to_rgb555(rgb, out555, 2, 2);
    printf("RGB555 : ");
    print_hex(out555, 8);

    unsigned char yuv[12];
    rgb888_to_yuv444(rgb, yuv, 2, 2);
    printf("YUV444 : ");
    print_hex(yuv, 12);

    unsigned char back[12];
    yuv444_to_rgb888(yuv, back, 2, 2);
    printf("RGB888 : ");
    print_hex(back, 12);

    return 0;
}
