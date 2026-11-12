/* input.h - asking the user questions while the program is running */
#ifndef INPUT_H
#define INPUT_H

#define MAX_LINE_LEN   256   /* longest text the user may type (file names too) */
#define MAX_ATTEMPTS   3     /* wrong answers allowed for each question         */

/* Each function returns the answer, or MODE_UNKNOWN / DIM_INVALID / FALSE
 * when the user gave up (too many wrong answers or the input ended). */
int ask_mode(void);
int ask_dimension(const char *label, int must_be_even);
int ask_input_path(char *path, int width, int height, int mode);
int ask_output_path(char *path, const char *input_path, int mode);

#endif /* INPUT_H */
