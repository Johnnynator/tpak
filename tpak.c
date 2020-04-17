#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

#include "tpak.h"

#if defined(_Win32) || defined(__MINGW32__)
#define mkdir(a,b) mkdir(a)
#endif

//#define HASH_EMIT_KEYS 3
#define HASH_FUNCTION HASH_BER
#include "uthash.h"

bool verbose = false;

struct name_entry {
	uint32_t length;
	char name;
};

struct file_entry {
	int32_t file_size;
	int32_t name_offset;
	int32_t chunk_count;
	int32_t chunk_index;
};

struct file_chunk_entry {
	int32_t unkwn;
	int32_t uncompressed_size;
	int32_t data_offset;
	int32_t compressed_size;
};

struct file_list {
	int index;
	char *name;
	struct fd_ll *file;
	UT_hash_handle hh;
};

struct fd_ll {
	struct fd_ll *next;
	FILE * file;
	struct file_entry *filetable;
	struct file_chunk_entry *chunktable;
	long header_end;
};

struct fd_ll *fd_ll_first = NULL;
struct fd_ll *fd_ll_last = NULL;
struct file_list *file_hh = NULL;

int t_uncompress(Bytef *input, int32_t in_size, Bytef *dest, int32_t dest_size) {
	int ret;
	z_stream zstream = {
		input,
		in_size,
		0,
		dest,
		dest_size,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		0,
		0
	};
	if(Z_OK != (ret = inflateInit2(&zstream, -15))) {
		return ret;
	}
	if (Z_STREAM_END == (ret = inflate(&zstream, Z_FINISH))) {
		return inflateEnd(&zstream);
	} else {
		if(zstream.msg != NULL)
			fprintf(stderr, "%s\n", zstream.msg);
		inflateEnd(&zstream);
		switch(ret) {
			case Z_MEM_ERROR:
				fprintf(stderr, "Z_MEM_ERROR\n");
				break;
			case Z_BUF_ERROR:
				fprintf(stderr, "Z_BUF_ERROR\n");
				break;
			case Z_DATA_ERROR:
				fprintf(stderr, "Z_DATA_ERROR\n");
				break;
			default:
				fprintf(stderr, "Unkown error\n");
				break;
		}
		return ret;
	}

}

void replace(char *str, char find, char replace) {
	for(size_t i = 0; i < strlen(str); i++) {
		if (str[i] == find) str[i] = replace;
	}
}

void decode_nametable(Bytef *nametable, uint32_t file_count) {
	for(size_t i = 0; i < file_count; i++) {
		struct name_entry * entry = (struct name_entry*)nametable;
		uint32_t mask = (entry->length % 5)+i;
		for(size_t j = 0; j < entry->length; j++) {
			*((&entry->name)+j) ^= (((j + entry->length)*2)+mask);
		}
		replace(&entry->name, '\\', '/'); // literally nothing expects \ as path seperator, even the game scripts expect / ...
		nametable += sizeof(uint32_t) + sizeof(char) * entry->length + 1;
	}
}

int readHeader(FILE * file) {
	Bytef* tmp = NULL;
	Bytef* src = NULL;
	Bytef* nametable = NULL;
	Bytef* filetable = NULL;
	struct file_chunk_entry* chunktable = NULL;
	int ret = 0;

	{
	uint32_t magic[2] = {0};
	if (fread(magic, 4, 2, file) != 2) {
		fprintf(stderr, "File truncated\n");
		ret = -3;
		goto RET;
	}
	if (magic[0] != 'KAPT') {
		fprintf(stderr, "\n");
		fprintf(stderr, "0x%.8X\n", magic[0]);
		ret = -1;
		goto RET;
	}
	if (magic[1] != 7) { // version
		fprintf(stderr, "%i\n", magic[1]);
		ret = -1;
		goto RET;
	}
	}
	fseek(file, 4l, SEEK_CUR); // dummy
	int32_t file_count = 0;
	if (fread(&file_count, 4, 1, file) != 1) {
		fprintf(stderr, "File truncated\n");
		ret = -3;
		goto RET;
	}
	fseek(file, 4l, SEEK_CUR);
	int32_t uncomp_nametable_size = 0;
	int32_t comp_nametable_size = 0;
	if (fread(&uncomp_nametable_size, 4, 1, file) != 1) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	if (fread(&comp_nametable_size, 4, 1, file) != 1) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
        }
	src = (Bytef*)malloc(comp_nametable_size);
	nametable = (Bytef*)malloc(uncomp_nametable_size);
	if (fread(src, 1, comp_nametable_size, file) != (long unsigned int)comp_nametable_size) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	*(int*)src ^= file_count;
	ret = t_uncompress(src, comp_nametable_size, nametable, uncomp_nametable_size);
	free(src);
	if(Z_OK != ret) {
		ret = -8;
		goto RET;
	}
	decode_nametable(nametable, file_count);
	tmp = nametable;
	for(int i = 0; i < file_count; i++) {
		struct name_entry * entry = (struct name_entry*)tmp;
		if(verbose) fprintf(stderr, "%i, %s\n", entry->length ,&(entry->name));
		tmp += sizeof(entry->length) + sizeof(entry->name) * entry->length +1;
	}
	fseek(file, (ftell(file)+3) & ~3, SEEK_SET);

	/*uint32_t* fileIndex = malloc(sizeof(uint32_t) * file_count);
	if (fread(fileIndex, 4, file_count, file) != (long unsigned int)file_count) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	if (verbose) for(int i = 0; i < file_count; i++) {
		fprintf(stderr, "%i\n", fileIndex[i]);
	}*/
	fseek(file, sizeof(uint32_t)*file_count, SEEK_CUR);

	int32_t uncomp_filetable_size = sizeof(struct file_entry) * file_count;
	int32_t comp_filetable_size = 0;
	if (fread(&comp_filetable_size, 4, 1, file) != 1) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
        }
	src = (Bytef*)malloc(comp_filetable_size);
	filetable = (Bytef*)malloc(uncomp_filetable_size);

	if (fread(src, 1, comp_filetable_size, file) != (long unsigned int)comp_filetable_size) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	if(verbose) fprintf(stderr, "0x%.16X\n", *(int*)src);
	*(int*)src ^= (file_count+comp_filetable_size);
	if(verbose) fprintf(stderr, "0x%.16X\n", *(int*)src);
	ret = t_uncompress(src, comp_filetable_size, filetable, uncomp_filetable_size);
	free(src);
	if(Z_OK != ret) {
		ret = -8;
		goto RET;
	}
	tmp = filetable;
	for( int i = 0; i < file_count; i++) {
		struct file_entry * entry = (struct file_entry*)tmp;
		if(verbose) fprintf(stderr ,"0x%.8X, 0x%.8X, 0x%.8X, 0x%.8X\n",
				entry->file_size,
				entry->chunk_count,
				entry->chunk_index,
				entry->name_offset);
		tmp += sizeof(struct file_entry);
	}
	fseek(file, (ftell(file)+3) & ~3, SEEK_SET);
	int32_t comp_chunktable_size = 0;
	int32_t chunk_count = 0;
	if (fread(&comp_chunktable_size, 4, 1, file) != 1) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	if (fread(&chunk_count, 4, 1, file) != 1) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	src = (Bytef*)malloc(comp_chunktable_size);
	chunktable = (struct file_chunk_entry*)malloc(chunk_count * sizeof(struct file_chunk_entry));

	if (fread(src, 1, comp_chunktable_size, file) != (long unsigned int)comp_chunktable_size) {
		fprintf(stderr, "file truncated\n");
		ret = -3;
		goto RET;
	}
	if(verbose) fprintf(stderr, "0x%.16X\n", *(int*)src);
	*(int*)src ^= (file_count+comp_chunktable_size+chunk_count);
	if(verbose) fprintf(stderr, "0x%.16X\n", *(int*)src);
	ret = t_uncompress(src, comp_chunktable_size, (Bytef*)chunktable, sizeof(struct file_chunk_entry)*chunk_count);
	free(src);
	if(Z_OK != ret) {
		ret = -8;
		goto RET;
	}
	for(int i = 0; i < file_count; i++) {
		if(verbose) fprintf(stderr, "Comp: 0x%.8X, Uncomp: 0x%.8X, Data: 0x%.8X, Unkwn: 0x%.8X\n",
				(chunktable+i)->compressed_size,
				(chunktable+i)->uncompressed_size,
				(chunktable+i)->data_offset,
				(chunktable+i)->unkwn);
	}
	fseek(file, (ftell(file)+3) & ~3, SEEK_SET);
	if(verbose) fprintf(stderr, "0x%.16lX\n", ftell(file));
	long header_end = ftell(file);
	fseek(file, 0, SEEK_END);
	if(verbose) fprintf(stderr, "0x%.16lX\n", ftell(file));
	struct fd_ll *fd_llist = malloc(sizeof(struct fd_ll));
	if(fd_ll_first == NULL) fd_ll_first = fd_llist;
	if(fd_ll_last != NULL) fd_ll_last->next = fd_llist;
	fd_llist->next = NULL;
	fd_llist->file = file;
	fd_llist->filetable = (struct file_entry*)filetable;
	fd_llist->chunktable = chunktable;
	fd_llist->header_end = header_end;
	fd_ll_last = fd_llist;
	struct name_entry * curr_name = (struct name_entry*)nametable;
	struct file_list * file_list = NULL;
	for (int j = 0; j < file_count; j++) {
		file_list = (struct file_list*)malloc(sizeof(*file_list));
		file_list->name = strdup(&curr_name->name);
		file_list->file = fd_llist;
		file_list->index = j;
		HASH_ADD_KEYPTR(hh, file_hh, file_list->name, strlen(file_list->name), file_list);
		Bytef* wtf = (Bytef *) curr_name;
		wtf += (sizeof(uint32_t) + sizeof(char) * curr_name->length + 1);
		curr_name = (struct name_entry*) wtf;
	}

	free(nametable);
	return ret;
RET:
	free(nametable);
	free(filetable);
	free(chunktable);
	return ret;
}

int tpak_free() {
	struct fd_ll * next_fd_ll = fd_ll_first;
	while(next_fd_ll != NULL) {
		struct fd_ll* curr_fd_ll = next_fd_ll;
		free(curr_fd_ll->chunktable);
		free(curr_fd_ll->filetable);
		fclose(curr_fd_ll->file);
		next_fd_ll = curr_fd_ll->next;
		free(curr_fd_ll);
	}
	fd_ll_first = NULL;
	fd_ll_last = NULL;
	struct file_list *entry, *tmp = NULL;
	HASH_ITER(hh, file_hh, entry, tmp) {
		HASH_DEL(file_hh, entry);
		free(entry->name);
		free(entry);
	}
	return 0;
}

void print_entry(struct file_list* entry, bool minimal) {
	if(verbose) {
		printf("File: %s\n", entry->name);
		printf("Data offset: 0x%.16lX\n", entry->file->header_end);
		printf("Chunk Count: %i, Index: %i, File size: %i, Name offset: 0x%.8X\n",
				entry->file->filetable[entry->index].chunk_count,
				entry->file->filetable[entry->index].chunk_index,
				entry->file->filetable[entry->index].file_size,
				entry->file->filetable[entry->index].name_offset);
		int * chunk_index = &entry->file->filetable[entry->index].chunk_index;
		for( int i = 0; i < entry->file->filetable[entry->index].chunk_count; i++) {
			fprintf(stdout, "Comp: 0x%.8X, Uncomp: 0x%.8X, Data: 0x%.8X, Unkwn: 0x%.8X\n",
		                        entry->file->chunktable[*chunk_index + i].compressed_size,
		                        entry->file->chunktable[*chunk_index + i].uncompressed_size,
		                        entry->file->chunktable[*chunk_index + i].data_offset,
		                        entry->file->chunktable[*chunk_index + i].unkwn);
		}
	} else if (minimal) {
		printf("%s\n", entry->name);
	} else {
		// Max filesize is limited by max size of int32_t (or maybe unsigned(?)), so limit
		// to length of 10 so we don't have to care about proper padding
		printf("%.10i\t%s\n", entry->file->filetable[entry->index].file_size, entry->name);
	}
}

int mkdir_p(char* dir) {
	char tmp = '\0';
	for(char * s = dir;;s++) {
		tmp = '\0';
		if(*s == '/') {
			tmp = *s;
			*s = '\0';
		} else if (*s) {
			continue;
		}
		if(mkdir(dir, 0755)) {
			if(errno != EEXIST) {
				return -1;
			}
		}
		if(tmp == '\0') return 0;
		*s = tmp;
	}
	return 0;
}

Bytef* get_chunk(struct file_list * entry, int i, Bytef* buffer_opt) {
	if(i < 0 || i > entry->file->filetable[entry->index].chunk_count) {
		return NULL;
	} else {
		Bytef * chunk = buffer_opt;
		int ret = 0;
		Bytef* src = NULL;
		int * chunk_index = &entry->file->filetable[entry->index].chunk_index;
		struct file_chunk_entry * curr_chunk_entry = &entry->file->chunktable[*chunk_index + i];
		if( chunk == NULL) {
			chunk = malloc(curr_chunk_entry->uncompressed_size);
		}
		fseek(entry->file->file, entry->file->header_end + curr_chunk_entry->data_offset, SEEK_SET);
		if(curr_chunk_entry->uncompressed_size == curr_chunk_entry->compressed_size) {
			if (fread(chunk, 1, curr_chunk_entry->compressed_size, entry->file->file) != (long unsigned int)curr_chunk_entry->compressed_size) {
				fprintf(stderr, "file truncated\n");
				return NULL;
			}
		} else {
			src = malloc(curr_chunk_entry->compressed_size);
			if (fread(src, 1, curr_chunk_entry->compressed_size, entry->file->file) != (long unsigned int)curr_chunk_entry->compressed_size) {
				fprintf(stderr, "file truncated\n");
				free(src);
				if(buffer_opt == NULL) free(chunk);
				return NULL;;
			}
			ret = t_uncompress(src, curr_chunk_entry->compressed_size, (Bytef*)chunk, curr_chunk_entry->uncompressed_size);
			free(src);
			if(Z_OK != ret) {
				if(buffer_opt == NULL) free(chunk);
				return NULL;
			}
		}
		return chunk;

	}
}

int extract_to_file(struct file_list *entry ,FILE* out) {
	Bytef *chunk = NULL;
	int *chunk_index = &entry->file->filetable[entry->index].chunk_index;
	for(int i = 0; i < entry->file->filetable[entry->index].chunk_count; i++) {
		struct file_chunk_entry * curr_chunk_entry = &entry->file->chunktable[*chunk_index + i];
		if((chunk = get_chunk(entry, i, NULL)) == NULL) {
			fclose(out);
			return -8;
		}
		//TODO: What is faster? Writing chunk by chunk or complete file at once
		fwrite(chunk, 1, curr_chunk_entry->uncompressed_size, out);
		free(chunk);
	}
	return 0;
}

int extract_to_dir(struct file_list *entry, const char *out_dir) {
	char *out_file = NULL;
	if(out_dir != NULL) {
		out_file = malloc(strlen(out_dir) + strlen(entry->name) + 2);
		strcpy(out_file, out_dir);
		if(out_file[strlen(out_dir)-1] != '/') {
			strcat(out_file, "/");
		}
		strcat(out_file, entry->name);
	} else {
		out_file = entry->name;
	}
	fprintf(stderr, "Extracting: %s\n", entry->name);
	char *out_cp = strdup(out_file);
	char *dir = dirname(out_cp);
	fprintf(stdout, "Out: %s\n", dir);
	FILE *out = fopen(out_file, "wb");
	if(out == NULL) {
		if(errno == ENOENT) {
			mkdir_p(dir);
			out = fopen(out_file, "wb");
			if(out == NULL) {
				perror("Extract to file:");
				free(out_cp);
				free(out_file);
				return -9;
			}
		}
	}
	free(out_file);
	free(out_cp);
	extract_to_file(entry, out);
	fclose(out);
	return 0;
}

//TODO: extract to memory
char * extract_entry(struct file_list *entry) {
	char* ret = malloc(entry->file->filetable[entry->index].file_size);
	Bytef * tmp = (Bytef*)ret;
	fprintf(stderr, "Extracting: %s\n", entry->name);
	int *chunk_index = &entry->file->filetable[entry->index].chunk_index;
	for(int i = 0; i < entry->file->filetable[entry->index].chunk_count; i++) {
		struct file_chunk_entry * curr_chunk_entry = &entry->file->chunktable[*chunk_index + i];
		if(get_chunk(entry, i, tmp) == NULL) {
			free(ret);
			return NULL;
		}
		tmp += curr_chunk_entry->uncompressed_size;
	}
	return ret;
}

struct file_list * find_file(const char * file) {
	struct file_list * ret;
	HASH_FIND_STR(file_hh, file, ret);
	return ret;
}

char * extract_file(const char * file, int32_t * size) {
	struct file_list *  entry = find_file(file);
	if(entry == NULL) {printf("Entry %s not found\n", file); return NULL;}
	*size = entry->file->filetable[entry->index].file_size;
	return extract_entry(entry);
}

int print_file(const char * file) {
	struct file_list *  entry = find_file(file);
	if(entry == NULL) {printf("Entry %s not found\n", file); return -1;}
	return extract_to_file(entry, stdout);
}

int read_dir(const char *path) {
	DIR * dir;
	FILE * t_fp;
	struct dirent * dire;
	char file[2048];
	int ret = 0;
	dir = opendir(path);
	if ( dir != NULL ) {
		while((dire = readdir(dir))) {
			if(strcmp(dire->d_name + strlen(dire->d_name) - 4, ".pak") == 0) {
				strcpy(file, path);
				if(file[strlen(file)-1] != '/') {
					strcat(file, "/");
				}
				strcat(file, dire->d_name);
				//puts (file);
				if((t_fp = fopen(file, "rb")) == NULL) {
					fprintf(stderr, "%s: %s\n", file, strerror(errno));
					ret = -1;
					continue;
				}
				readHeader(t_fp);
			}
		}
		closedir(dir);
	}
	return ret;
}

int read_input(const char *path) {
	FILE *fp = NULL;
	struct stat stat_buf;
	if(stat(path, &stat_buf) == 0) {
		if(S_ISDIR(stat_buf.st_mode)) {
			return read_dir(path);
		} else {
			if ((fp = fopen(path, "rb")) != NULL)
				return readHeader(fp);
			return -1;
		}
	} else {
		fprintf(stderr, "%s: %s\n", path, strerror(errno));
		return -1;
	}
}

int extract_all(const char *path, const char *out, bool allow_failure) {
	read_input(path);
	struct file_list *entry, *tmp = NULL;
	HASH_ITER(hh, file_hh, entry, tmp) {
		if((extract_to_dir(entry, out) != 0) && !allow_failure) {
				return -1;
		}
	}
	return 0;
}

void print_all(bool minimal) {
	struct file_list *entry, *tmp = NULL;
	HASH_ITER(hh, file_hh, entry, tmp) {
		print_entry(entry, minimal);
	}
}
