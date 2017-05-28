/*
 * main.c
 *
 *  Created on: 28.10.2014
 *      Author: pascal
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[])
{
	struct dirent **dirents;
	const char *arg;

	if(argc == 1)
	{
		arg = getenv("PWD");
		if(arg == NULL)
		{
			fputs("Could not list current directory\n", stderr);
			return EXIT_FAILURE;
		}
	}
	else
	{
		arg = argv[1];
	}

	int size = scandir(arg, &dirents, NULL, alphasort);
	if(size == -1)
	{
		fprintf(stderr, "Could not list %s\n", arg);
		return EXIT_FAILURE;
	}
	else
	{
		for(int i = 0; i < size; i++)
		{
			char type;
			switch(dirents[i]->d_type)
			{
				case DT_DIR:
					type = 'd';
				break;
				case DT_FILE:
					type = 'f';
				break;
				case DT_LINK:
					type = 'l';
				break;
				case DT_DEV:
					type = 'c';
				break;
				default:
					type = ' ';
			}
			printf("%c\t%s", type, dirents[i]->d_name);
			if(dirents[i]->d_type == DT_FILE)
			{
				static const char *units[] = {
					"B", "KiB", "MiB", "GiB", "TiB"
				};
				char *filepath = malloc(strlen(arg) + strlen(dirents[i]->d_name) + 1);
				if(filepath != NULL)
				{
					strcpy(filepath, arg);
					strcat(filepath, dirents[i]->d_name);
					FILE *fp = fopen(filepath, "rb");
					if(fp != NULL)
					{
						fseek(fp, 0, SEEK_END);
						double filesize = ftell(fp);
						fclose(fp);
						size_t i = 0;
						while(filesize >= 1024 && i < sizeof(units) / sizeof(units[0]) - 1)
						{
							filesize /= 1024;
							i++;
						}
						printf("\t%.2f%s", filesize, units[i]);
					}
					free(filepath);
				}
			}
			putchar('\n');
			free(dirents[i]);
		}
		free(dirents);
	}
	return EXIT_SUCCES;
}
