/* file_io.h - reading and writing raw files */
#ifndef FILE_IO_H
#define FILE_IO_H

/* Returned by get_file_size() when the file cannot be opened */
#define FILE_SIZE_ERROR     (-1)

int  file_exists(const char *path);

long get_file_size(const char *path);
int  read_file(const char *path, unsigned char *buffer, long size);
int  write_file(const char *path, const unsigned char *buffer, long size);

#endif /* FILE_IO_H */
