#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int readHeader(FILE * file);
int extract_dir(char *path, char *out, bool allow_failure);
char * extract_file(const char * file, int32_t * size);
int read_dir(char *path);
void print_all();
int tpak_free();
