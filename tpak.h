#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int readHeader(FILE * file);
int extract_all(const char *path, const char *out, bool allow_failure);
char * extract_file(const char * file, int32_t * size);
int read_dir(const char *path);
int read_input(const char *input);
int print_file(const char *file);
void print_all();
int tpak_free();
