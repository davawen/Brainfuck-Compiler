#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

long long get_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

void draw_memory(WINDOW *local_win, int *memory, int *ptr, int w)
{
	size_t offset = ptr-memory;
	
	for(int i = 1; i < w-1; i += 5)
	{
		mvwprintw(local_win, 1, i, "|%-4i", memory[i / 5]);
		mvwaddch(local_win, 2, i + 2, ' ');
	}
	
	if(offset * 5 + 3 < w) mvwaddch(local_win, 2, offset * 5 + 3, '^');
	
	box(local_win, 0, 0);
}

bool is_default(char chr)
{
	const char *chars = "><+-.,[]";

	return strchr(chars, chr) == NULL;
}

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		printf("Error: no file specified.\n");
		exit(-1);
	}
	
	FILE *fp = fopen(argv[1], "r");

	if(fp == NULL)
	{
		printf("Error: Couldn't open file.\n");
		exit(-1);
	}
	
	initscr();

	raw();
	keypad(stdscr, TRUE);
	noecho();
	nodelay(stdscr, TRUE);
	curs_set(FALSE);
	
	char filebuffer[128000];
	
	{
		char *bufPtr = filebuffer;
		
		char chr;
		while((chr = fgetc(fp)) != EOF)
		{
			*bufPtr = chr;
			
			bufPtr++;
		}
		
		*bufPtr = '\0';
		
		fseek(fp, 0, SEEK_SET);
	}
	
	int width, height;
	
	getmaxyx(stdscr, height, width);
	
	int memoryWidth = min(80, width / 2);
	int fileWidth = width - memoryWidth;
	
	WINDOW *file_win = newwin(20, fileWidth, 0, memoryWidth);
	
	refresh();
	
	box(file_win, 0, 0);
	
	{
		char *ptr = filebuffer;
		
		int lineNum = 1;
		int lineOffset = 1;
		
		while(*ptr != '\0')
		{
			mvwprintw(file_win, lineOffset, 1, "%-3i ", lineNum);
			
			int offset = 5;
			while(*ptr != '\n' && *ptr != '\0')
			{
				mvwaddch(file_win, lineOffset, offset, *ptr);
				
				offset++;
				ptr++;
				
				if(offset >= fileWidth)
				{
					lineOffset++;
					offset = 5;
				}
			}	
			
			if(*ptr != '\0') ptr++;
			
			lineNum++;
			lineOffset++;
		}
	}
	
	wrefresh(file_win);
	refresh();
	
	WINDOW *memory_win = newwin(4, memoryWidth, 0, 0);
	WINDOW *output_win = newwin(20, memoryWidth, 4, 0);
	
	refresh();
	
	wmove(output_win, 0, 0);
	nodelay(output_win, FALSE);
	
	int memory[30000];
	
	int *ptr = memory;
	
	int layer = 0;
	
	size_t speed = 10000;
	bool fullspeed = false;
	bool paused = true;
	
	int lineOffset = 1;
		
	int offset = 5;
	
	int endOffset[999];
	
	char chr;
	while((chr = fgetc(fp)) != EOF)
	{
		if(!is_default(chr))
		{
			draw_memory(memory_win, memory, ptr, 80);

			wchgat(output_win, 1, A_REVERSE, 0, NULL);

			mvwchgat(file_win, lineOffset, offset, 1, A_REVERSE, 0, NULL);

			wrefresh(memory_win);
			wrefresh(file_win);
			wrefresh(output_win);
			refresh();

			long long start = get_time();

			while(get_time() - start < speed || paused)
			{
				int ch = getch();

				if(ch == 'q') goto end;
				else if(ch == 'k') fullspeed = !fullspeed;
				else if(ch == ' ') paused = !paused;
				else if(ch == KEY_RIGHT) speed /= 1.5;
				else if(ch == KEY_LEFT) speed *= 1.5;

				if(fullspeed && (get_time() - start < 5)) break;
			}

			mvwchgat(file_win, lineOffset, offset, 1, A_NORMAL, 0, NULL);

			bool wasDefault = false;
			switch(chr)
			{
				case '>':
					ptr++;
					break;
				case '<':
					ptr--;
					break;
				case '+':
					(*ptr)++;
					break;
				case '-':
					(*ptr)--;
					break;
				case '.':
					waddch(output_win, *ptr);
					break;
				case ',':
					*ptr = wgetch(output_win);
					break;
				case '[':
					if(*ptr != 0) break;

					while((chr = fgetc(fp)) != EOF)
					{
						if(chr == '[') layer++;
						else if(chr == ']')
						{
							if(layer == 0) break;
							else layer--;
						}

						offset++;

						if(offset >= fileWidth)
						{
							lineOffset++;
							offset = 5;
						}

						if(chr == '\n')
						{
							lineOffset++;

							offset = 5;
						}
					}
					break;
				case ']':
					if(*ptr == 0) break;

					for(;;)
					{
						fseek(fp, -2L, SEEK_CUR);
						if(ftell(fp) < 0)
						{
							printf("Error: no opening bracket specified");
							exit(-1);
						}

						chr = fgetc(fp);

						offset--;

						if(chr == ']') layer++;
						else if(chr == '[')
						{
							if(layer == 0) break;
							else layer--;
						}

						if(offset <= 0)
						{
							lineOffset--;
							offset = endOffset[lineOffset];
						}

						if(chr == '\n')
						{
							lineOffset--;

							offset = endOffset[lineOffset];
						}
					}
					break;
				default:
					break;
			}
		}
		
		offset++;
		
		if(offset >= fileWidth)
		{
			endOffset[lineOffset] = offset-1;
			
			lineOffset++;
			offset = 5;
		}
		
		if(chr == '\n')
		{
			endOffset[lineOffset] = offset - 1;
			
			lineOffset++;
			
			offset = 5;
		}
		
	}
	
	while(getch() != 'q');

	end:
	
	fclose(fp);	
	
	delwin(file_win);
	delwin(memory_win);
	endwin();
	
	return 0;
}
