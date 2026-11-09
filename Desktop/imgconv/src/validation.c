/* validation.c - checking numbers typed by the user */
#include <stdio.h>
#include <stdlib.h>
#include "validation.h"
#include "common.h"

#define END_OF_STRING       '\0'
#define DIGIT_FIRST         '0'
#define DIGIT_LAST          '9'

/*
 * Return TRUE if text contains only digits (1 to MAX_DIM_DIGITS of them).
 * Limiting the length stops atoi() from overflowing on huge numbers.
 * A minus sign or letters make it fail, so "-5" and "abc" are rejected.
 */
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
