/* validation.h - checking numbers typed by the user */
#ifndef VALIDATION_H
#define VALIDATION_H

#define MIN_DIM             1       /* smallest width/height accepted     */
#define MAX_DIM             8192    /* biggest width/height accepted      */
#define MAX_DIM_DIGITS      5       /* longest number text we even try    */
#define DIM_NOT_SET         0       /* width/height not given yet         */
#define DIM_INVALID         (-1)    /* returned by get_dimension on error */

int is_number(const char *text);
int get_dimension(const char *text, const char *name);

int has_extension(const char *path, const char *extension);

#endif /* VALIDATION_H */
