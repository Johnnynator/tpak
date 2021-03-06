#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "tpak.h"
#ifdef HAVE_LUAJIT
#include "lua.h"
#endif

extern bool verbose;

// right now not really guessing, just a single hardcoed path
char *guess_path() {
	char *path = "/.local/share/Steam/steamapps/common/star conflict/data/";
	char *home = NULL;
	if ((home = getenv("HOME")) == NULL)
		return NULL;
	char *ret = malloc(strlen(path) + strlen(home) + 1);
	strcpy(ret, home);
	strcat(ret, path);
	return ret;
}

void usage(char* argv0, int ret) {
	fprintf(stdout,
		"Usage: %s [OPTIONS]\n\n"
		"OPTIONS\n"
		" -i  --in <dir>    Path to game dir (default to Steam installation in ~/.local/share\n"
		" -o  --out <dir>   Path to outpit dir (default to ./)\n"
		" -l  --list        Only list files, do no extract anything\n"
		" -f  --file <path> Only extract a single file\n"
		" -c  --console     launch lua console\n"
		" -v  --verbose     Enable verbose output\n"
		" -h  --help        Print help\n", basename(argv0));
	exit(ret);
}

int main(int argc, char **argv) {
	bool list = false, console = false;
	int c;
	char *input = NULL;
	char *outdir = NULL;
	char *file = NULL;
	const char * options = ":i:vo:lhf:c";
	const struct option longoptions[] = {
		{ "in", required_argument, NULL, 'i' },
		{ "out", required_argument, NULL, 'o' },
		{ "file", required_argument, NULL, 'f' },
		{ "list", no_argument, NULL, 'l' },
		{ "verbose", no_argument, NULL, 'v'},
		{ "help", no_argument, NULL, 'h' },
		{ "console", no_argument, NULL, 'c' }
	};
	while((c = getopt_long(argc, argv, options, longoptions, NULL)) != -1) {
		switch(c) {
			case 'i':
				input = optarg;
				break;
			case 'o':
				outdir = optarg;
				break;
			case 'f':
				file = optarg;
				break;
			case 'h':
				usage(argv[0], EXIT_SUCCESS);
				break;
			case 'v':
				verbose = true;
				break;
			case 'l':
				list = true;
				break;
			case 'c':
				console = true;
				break;
			default:
				usage(argv[0], EXIT_FAILURE);
				break;
		}
	}
	if(input == NULL) {
		// not freed anywhere so far
		if((input = guess_path()) == NULL) {
			fprintf(stderr, "Cannon guess data path, please manual provide one with --in/-i <path>\n");
			exit(EXIT_FAILURE);
		}
	}
	if(file != NULL) {
		read_input(input);
		print_file(file);
	} else if(list) {
		read_input(input);
		print_all(true);
	} else if (console) {
#ifdef HAVE_LUAJIT
		read_input(input);
		if(initLua() == 0) luaConsole();
		closeLua();
#else
		fprintf(stderr, "Compiled without LuaJIT");
#endif
	} else {
		extract_all(input, outdir, true);
	}
	tpak_free();
}
