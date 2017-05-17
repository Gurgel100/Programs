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

	while(1) syscall_sleep(1000); //Dieser Prozess darf nicht beendet werden
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
				//Newline am Ende entfernen
				size_t buflen = strlen(buffer);
				if(buffer[buflen - 1] == '\n')
					buffer[buflen - 1] = '\0';
				printf("read: %s\n", buffer);
				if(i == 0)
				{
					//Pfad einlesen
					binpath = strdup(buffer);
				}
				else if(i == 1)
				{
					printf("Created %lu on this console\n", createProcess(binpath, buffer, NULL, NULL, NULL, NULL));
				}
				else
				{
					char *console;
					asprintf(&console, "/dev/tty%.2lu", i);
					printf("Created %lu on new console\n", createProcess(binpath, buffer, NULL, console, console, console));
					free(console);
				}
			}
			else if(!feof(fp))
			{
				printf("Es ist ein Fehler beim Lesen der Datei aufgetreten\n");
				break;
			}
			i++;
		}
	}
failure:
	free(buffer);
}
