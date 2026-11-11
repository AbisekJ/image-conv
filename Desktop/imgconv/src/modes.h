/* modes.h - the list of conversions and the code that picks one */
#ifndef MODES_H
#define MODES_H

/* Number given to each conversion */
#define MODE_UNKNOWN            0
#define MODE_RGB888_TO_RGB565   1
#define MODE_RGB888_TO_RGB555   2
#define MODE_RGB888_TO_YUV444   3
#define MODE_YUV444_TO_RGB888   4
#define MODE_YUV444_TO_YUV422   5
#define MODE_YUV444_TO_YUV420   6

/* Valid menu numbers */
#define MODE_FIRST              1
#define MODE_LAST               6

void print_mode_menu(void);
int  needs_even_width(int mode);
int  needs_even_height(int mode);

//int  get_mode(const char *name);
long get_output_size(int mode, int width, int height);
void convert(int mode, const unsigned char *input, unsigned char *output,
             int width, int height);

#endif /* MODES_H */
