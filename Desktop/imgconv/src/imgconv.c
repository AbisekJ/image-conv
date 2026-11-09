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


void rgb888_to_rgb565(const unsigned char *in, unsigned char *out, int width, int height)
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

    return 0;
}
