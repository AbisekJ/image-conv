/* file_io.c - reading and writing raw files */
#include <stdio.h>
#include "file_io.h"
#include "common.h"

#define NO_OFFSET           0   /* fseek offset: measure from SEEK_END exactly */
#define BYTE_SIZE           1   /* fread/fwrite item size: one byte at a time  */

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
