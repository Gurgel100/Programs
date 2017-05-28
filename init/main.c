/*
 * main.c
 *
 *  Created on: 04.11.2014
 *      Author: pascal
 */

#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "math.h"
#include "userlib.h"
#include "syscall.h"
#include <ctype.h>

char *binpath;

void parseFile(FILE *fp);

int main(int argc, char *argv[])
{
	FILE *fp;

	fp = fopen("/init.ini", "rb");
	if(fp == NULL)
		printf("Konnte keine Init-Datei laden\n");
	else
	{
		parseFile(fp);
	}

	fclose(fp);

	while(1) syscall_wait(0, NULL);	//Dieser Prozess darf nicht beendet werden
}

static char *trim(char *str)
{
	//Skip whitespaces at the beginning
	while(isspace(*str)) str++;

	//Delete whitespces at the end
	size_t str_len = strlen(str);
	if(str_len > 0)
	{
		char *end = str + str_len - 1;
		while(isspace(*end))
			*end-- = '\0';
	}

	return str;
}

void parseFile(FILE *fp)
{
	uint64_t filesize;
	char *buffer;

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer = malloc(filesize);
	if(buffer == NULL)
		printf("Es konnte kein Speicher angefordert werden\n");
	else
	{
		size_t i = 0;
		while(!feof(fp))
		{
			if(fgets(buffer, filesize, fp) != NULL)
			{
				//Remove whitespace at the beginning and at the of the line
				char *str = trim(buffer);

				//Ignore comments
				if(*str == '#')
					continue;

				if(strncmp(str, "ENV", 3) == 0 && isspace(str[3]))
				{
					char *name = trim(str + 4);
					char *value = strchr(name, '=');

					//TODO: error message
					if(value == NULL || name >= value)
						continue;

					*value = '\0';
					setenv(name, value + 1, 1);
				}
				else
				{
					if(i == 0)
					{
						//Pfad einlesen
						binpath = strdup(buffer);
					}
					else if(i == 1)
					{
						printf("Created %lu on this console\n", createProcess(NULL, buffer, NULL, NULL, NULL, NULL));
					}
					else
					{
						char *console;
						asprintf(&console, "/dev/tty%.2lu", i);
						printf("Created %lu on new console\n", createProcess(NULL, buffer, NULL, console, console, console));
						free(console);
					}
					i++;
				}
			}
			else if(!feof(fp))
			{
				printf("Es ist ein Fehler beim Lesen der Datei aufgetreten\n");
				break;
			}
		}
	}
	free(buffer);
}
