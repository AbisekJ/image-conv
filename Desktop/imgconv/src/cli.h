/* cli.h - text printed for --help, --version and usage */
#ifndef CLI_H
#define CLI_H

#define PROGRAM_NAME        "imgconv"
#define VERSION             "1.0.0"

/* Command line option names */
#define OPT_HELP_SHORT      "-h"
#define OPT_HELP_LONG       "--help"
#define OPT_VERSION_SHORT   "-v"
#define OPT_VERSION_LONG    "--version"

void print_usage(void);
void print_version(void);
void print_help(void);

#endif /* CLI_H */
