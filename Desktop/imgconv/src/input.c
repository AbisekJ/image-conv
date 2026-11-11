/* input.c - asking the user questions while the program is running */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "common.h"
#include "file_io.h"
#include "image_convert.h"
#include "modes.h"
#include "validation.h"

/* Results of read_line() */
#define READ_OK        1
#define READ_EOF       0
#define READ_TOO_LONG  (-1)

/* Results of ask_yes_no() */
#define ANSWER_YES     1
#define ANSWER_NO      0
#define ANSWER_ERROR   (-1)

#define CHAR_NEWLINE   '\n'
#define CHAR_RETURN    '\r'
#define ANSWER_YES_LOWER  'y'
#define ANSWER_YES_UPPER  'Y'
#define ANSWER_NO_LOWER   'n'
#define ANSWER_NO_UPPER   'N'

/*
 * Read one line typed by the user into buffer (without the newline).
 * fgets() never writes more than 'size' characters, so the buffer cannot overflow.
 */
static int read_line(char *buffer, int size)
{
    int length;
    int ch;

    fflush(stdout);                         /* show the question first */
    if (fgets(buffer, size, stdin) == NULL) {
        fprintf(stderr, "\nError: no more input\n");
        return READ_EOF;
    }
    length = (int)strlen(buffer);

    if (length > 0 && buffer[length - 1] == CHAR_NEWLINE) {
        buffer[length - 1] = END_OF_STRING;             /* remove the newline */
        length--;
    } else if (length == size - 1) {
        /* buffer is full and there was no newline: the line was too long */
        while ((ch = getchar()) != CHAR_NEWLINE && ch != EOF) {
            /* throw away the rest of the line */
        }
        fprintf(stderr, "Error: input is too long (maximum %d characters)\n", size - 1);
        return READ_TOO_LONG;
    }
    if (length > 0 && buffer[length - 1] == CHAR_RETURN) {
        buffer[length - 1] = END_OF_STRING;             /* remove Windows \r */
    }
    return READ_OK;
}

/* Ask a yes/no question. Return ANSWER_YES, ANSWER_NO or ANSWER_ERROR. */
static int ask_yes_no(const char *question)
{
    char line[MAX_LINE_LEN];
    int attempt;

    for (attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        int result;

        printf("%s (y/n): ", question);
        result = read_line(line, MAX_LINE_LEN);
        if (result == READ_EOF) {
            return ANSWER_ERROR;
        }
        if (result == READ_TOO_LONG) {
            continue;
        }
        if (line[1] == END_OF_STRING &&
            (line[0] == ANSWER_YES_LOWER || line[0] == ANSWER_YES_UPPER)) {
            return ANSWER_YES;
        }
        if (line[1] == END_OF_STRING &&
            (line[0] == ANSWER_NO_LOWER || line[0] == ANSWER_NO_UPPER)) {
            return ANSWER_NO;
        }
        fprintf(stderr, "Error: please type y or n\n");
    }
    fprintf(stderr, "Error: too many wrong answers\n");
    return ANSWER_ERROR;
}

/* Show the menu and ask for a mode number. */
int ask_mode(void)
{
    char line[MAX_LINE_LEN];
    int attempt;

    print_mode_menu();
    for (attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        int result;
        int choice;

        printf("Enter mode number (%d to %d): ", MODE_FIRST, MODE_LAST);
        result = read_line(line, MAX_LINE_LEN);
        if (result == READ_EOF) {
            return MODE_UNKNOWN;
        }
        if (result == READ_TOO_LONG) {
            continue;
        }
        if (!is_number(line)) {
            fprintf(stderr, "Error: please type a number from %d to %d\n",
                    MODE_FIRST, MODE_LAST);
            continue;
        }
        choice = atoi(line);
        if (choice < MODE_FIRST || choice > MODE_LAST) {
            fprintf(stderr, "Error: mode number must be from %d to %d, got %d\n",
                    MODE_FIRST, MODE_LAST, choice);
            continue;
        }
        return choice;
    }
    fprintf(stderr, "Error: too many wrong answers\n");
    return MODE_UNKNOWN;
}

/*
 * Ask for the width or the height (label says which one).
 * must_be_even is TRUE when the chosen mode needs an even number.
 */
int ask_dimension(const char *label, int must_be_even)
{
    char line[MAX_LINE_LEN];
    int attempt;

    for (attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        int result;
        int value;

        printf("Enter image %s in pixels (%d to %d): ", label, MIN_DIM, MAX_DIM);
        result = read_line(line, MAX_LINE_LEN);
        if (result == READ_EOF) {
            return DIM_INVALID;
        }
        if (result == READ_TOO_LONG) {
            continue;
        }
        value = get_dimension(line, label);         /* prints its own error */
        if (value == DIM_INVALID) {
            continue;
        }
        if (must_be_even && value % CHROMA_STEP != 0) {
            fprintf(stderr, "Error: this mode needs an even %s, got %d\n", label, value);
            continue;
        }
        return value;
    }
    fprintf(stderr, "Error: too many wrong answers\n");
    return DIM_INVALID;
}

/* Ask for the input file name. The file must exist and have the right size. */
int ask_input_path(char *path, int width, int height)
{
    long expected_size = (long)width * height * INPUT_BYTES_PER_PIXEL;
    int attempt;

    for (attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        int result;
        long actual_size;

        printf("Enter input file name: ");
        result = read_line(path, MAX_LINE_LEN);
        if (result == READ_EOF) {
            return FALSE;
        }
        if (result == READ_TOO_LONG) {
            continue;
        }
        if (path[0] == END_OF_STRING) {
            fprintf(stderr, "Error: file name cannot be empty\n");
            continue;
        }
        actual_size = get_file_size(path);
        if (actual_size == FILE_SIZE_ERROR) {
            fprintf(stderr, "Error: input file '%s' not found or cannot be opened\n", path);
            continue;
        }
        if (actual_size != expected_size) {
            fprintf(stderr, "Error: input file is %ld bytes, expected %ld (%d x %d x %d)\n",
                    actual_size, expected_size, width, height, INPUT_BYTES_PER_PIXEL);
            continue;
        }
        return TRUE;
    }
    fprintf(stderr, "Error: too many wrong answers\n");
    return FALSE;
}

/* Ask for the output file name. If it already exists, ask before overwriting. */
int ask_output_path(char *path, const char *input_path)
{
    int attempt;

    for (attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        int result;

        printf("Enter output file name: ");
        result = read_line(path, MAX_LINE_LEN);
        if (result == READ_EOF) {
            return FALSE;
        }
        if (result == READ_TOO_LONG) {
            continue;
        }
        if (path[0] == END_OF_STRING) {
            fprintf(stderr, "Error: file name cannot be empty\n");
            continue;
        }
        if (strcmp(path, input_path) == 0) {
            fprintf(stderr, "Error: input and output must be different files\n");
            continue;
        }
        if (file_exists(path)) {
            int answer = ask_yes_no("File already exists. Do you wish to overwrite it?");

            if (answer == ANSWER_ERROR) {
                return FALSE;
            }
            if (answer == ANSWER_NO) {
                continue;                           /* ask for another name */
            }
        }
        return TRUE;
    }
    fprintf(stderr, "Error: too many wrong answers\n");
    return FALSE;
}
