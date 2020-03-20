#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tpak.h"

extern bool verbose;

// right now not really guessing, just a single hardcoed path
char *guess_path() {
	char *path = "/.local/share/Steam/steamapps/common/star conflict/data/";
	char *home = getenv("HOME");
	char *ret = malloc(strlen(path) + strlen(home) + 1);
	strcpy(ret, home);
	strcat(ret, "/.local/share/Steam/steamapps/common/star conflict/data/");
	return ret;
}

void usage(char* argv0) {
	fprintf(stdout,
		"Usage: %s [OPTIONS]\n\n"
		"OPTIONS\n"
		" -i  --in <dir>  Path to game dir (default to Steam installation in ~/.local/share\n"
		" -o  --out <dir>  Path to outpit dir (default to ./\n"
		" -l  --list       Only list files, do no extract anything\n"
		" -v  --verbose    Enable verbose output\n"
		" -h  --help       Print help\n", basename(argv0));
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	bool list = false;
	int c;
	char *indir = NULL;
	char *outdir = NULL;
	const char * options = "ivolh";
	const struct option longoptions[] = {
		{ "in", required_argument, NULL, 'i' },
		{ "out", required_argument, NULL, 'o' },
		{ "list", no_argument, NULL, 'l' },
		{ "verbose", no_argument, NULL, 'v'},
		{ "help", no_argument, NULL, 'h' }
	};
	while((c = getopt_long(argc, argv, options, longoptions, NULL)) != -1) {
		switch(c) {
			case 'i':
				indir = optarg;
				break;
			case 'o':
				outdir = optarg;
				break;
			case 'h':
				usage(argv[0]);
				break;
			case 'v':
				verbose = true;
				break;
			case 'l':
				list = true;
				break;
		}
	}
	if(indir == NULL) {
		// not freed anywhere so far
		indir = guess_path();
	}
	if(list) {
		read_dir(indir);
		print_all();
	} else {
		extract_dir(indir, outdir);
	}
	tpak_free();
}
