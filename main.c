#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
	
	char memory[30000];
	
	char *ptr = memory;
	
	int layer = 0;
	
	char chr;
	while((chr = fgetc(fp)) != EOF)
	{
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
				putchar(*ptr);
				break;
			case ',':
				*ptr = getchar();
				break;
			case '[':
				if(*ptr != 0) continue;
				
				while((chr = fgetc(fp)) != EOF)
				{
					if(chr == '[') layer++;
					else if(chr == ']')
					{
						if(layer == 0) break;
						else layer--;
					}
				}
				break;
			case ']':
				if(*ptr == 0) continue;
				
				for(;;)
				{
					fseek(fp, -2L, SEEK_CUR);
					if(ftell(fp) < 0)
					{
						printf("Error: no opening bracket specified");
						exit(-1);
					}
					
					chr = fgetc(fp);
					
					if(chr == ']') layer++;
					else if(chr == '[')
					{
						if(layer == 0) break;
						else layer--;
					}
				}
				break;
			default:
				break;
		}
	}
	
	fclose(fp);
	
	printf("\n"); // Padding
	
	return 0;
}