#include <stdio.h>
#include <stdlib.h>

#define check_malloc(ptr) do { \
	if(ptr == NULL) { \
		printf("failed to allocate %p\n", (void *)ptr); \
		exit(-1); \
	} \
	} while(0);

#define MAX_DEPTH 128

typedef struct {
	char *ptr;
	size_t length;
	size_t reserved;
} File;

File read_file(const char *filename) {
	FILE *fp = fopen(filename, "r");

	if(fp == NULL) {
		printf("Couldn't open file %s\n", filename);
		exit(-1);
	}

	File file;

	file.ptr = (char *)malloc(256);

	check_malloc(file.ptr);

	file.reserved = 256;
	file.length = 0;

	char c;

	while(1) {
		c = fgetc(fp);

		if(c == EOF) c = '\0';

		file.ptr[file.length++] = c;

		if(c == '\0') break;

		if(file.length == file.reserved) {
			file.ptr = (char *)realloc(file.ptr, file.reserved * 2);
			check_malloc(file.ptr);

			file.reserved *= 2;
		}
	}

	fclose(fp);

	return file;
}

void close_file(File file) {
	free(file.ptr);
	file.length = 0;
	file.reserved = 0;
}

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("Usage: compiler {filename}\n");
		exit(-1);
	}

	File file = read_file(argv[1]);

	FILE *fp = fopen("a.s", "w");

#define out(...) fprintf(fp, __VA_ARGS__);

	out("global _start\n");
	out("section .text\n\n");

#define M_PUTC 


	out("_start:\n\n");

	// Initialize memory to zero
	out("\tmov rax, rsp\n");
	out("\tmov rcx, 10000\n");
	out("\t.loop:\n");
	out("\tmov BYTE [rsp], 0\n");
	out("\tsub rsp, 1\n");
	out("\tdec rcx\n");
	out("\tjnz .loop\n");
	out("\tmov rsp, rax\n\n");

	size_t bracket_index[MAX_DEPTH] = { 0 }; // each depth needs a new identifier

	size_t bracket_depth = 0; 

	for(size_t i = 0; i < file.length; i++) {
		switch(file.ptr[i]) {
			case '+':
				out("\tinc BYTE [rsp]\n");
				break;
			case '-':
				out("\tdec BYTE [rsp]\n");
				break;
			case '>':
				out("\tsub rsp, 1\n");
				break;
			case '<':
				out("\tadd rsp, 1\n");
				break;
			case '.':
				out("\tmov rdi, 1 ; stdout\n")
				out("\tmov rsi, rsp ; ptr to string\n")
				out("\tmov rdx, 1 ; msg length\n")
				out("\tmov rax, 1 ; sys_write()\n")
				out("\tsyscall\n\n")
				break;
			case ',':
				out("\tmov rdi, 0 ; stdin\n")
				out("\tmov rsi, rsp ; ptr to string\n")
				out("\tmov rdx, 1 ; msg length\n")
				out("\tmov rax, 0 ; sys_read()\n")
				out("\tsyscall\n\n")
				break;
			case '[':

				// If current byte is 0, jump to matching ]
				out("\tcmp BYTE [rsp], 0\n");
				out("\tje closing_bracket_%zu_%zu\n", bracket_depth, bracket_index[bracket_depth]);
				out("\topening_bracket_%zu_%zu:\n\n", bracket_depth, bracket_index[bracket_depth]);

				bracket_depth++;
				break;
			case ']':
				bracket_depth--;

				// If current byte is non zero, jump to matching [
				out("\tcmp BYTE [rsp], 0\n");
				out("\tjne opening_bracket_%zu_%zu\n", bracket_depth, bracket_index[bracket_depth]);
				out("\tclosing_bracket_%zu_%zu:\n", bracket_depth, bracket_index[bracket_depth]);

				bracket_index[bracket_depth]++;
				break;
			default:
				break;
		}
	}

	out("\tmov rax, 60\n");
	out("\txor rdi, rdi\n");
	out("\tsyscall\n");

#undef out

	fclose(fp);

	close_file(file);

	system("nasm -felf64 a.s && ld a.o");
}
