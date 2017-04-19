/*
 * main.c
 *
 *  Created on: 28.10.2014
 *      Author: pascal
 */

#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "userlib.h"
#include "math.h"
#include "dirent.h"
#include "string.h"
#include "ctype.h"
#include "syscall.h"

typedef struct{
	const char *cmd;
	const char *desc;
	void (*func)(const char*);
}command_t;

void command(char *cmd);
double c;

static void help();
static void info();
static void ls(const char*);
static void cat(const char*);
static void mount(const char*);
static void unmount(const char*);
static const command_t commands[] = {
		{"help", "Gibt diese Hilfe aus", help},
		{"info", "Gibt Systeminformationen aus", info},
		{"ls", "Listet den Inhalt eines Verzeichnisses auf", ls},
		{"cat", "Gibt den Inhalt einer Datei aus", cat},
		{"mount", "Mountet ein Dateisystem", mount},
		{"umount", "Unmountet ein Dateisystem", unmount},
		{NULL, NULL, NULL}
};

int main(int argc, char *argv[])
{
	size_t cmd_size = 0;
	char *cmd = calloc(cmd_size + 1, 1);
	printf("Willkommen bei YourOS V0.1\n");
	putchar('\n');
	putchar('>');
	//Einfache ein-/ausgabe
	while(true)
	{
		char c = getchar();
		switch(c)
		{
			case '\n':
				putchar(c);
				command(cmd);
				putchar('>');
				cmd = realloc(cmd, 1);
				cmd[0] = '\0';
				cmd_size = 0;
			break;
			case '\b':
				if(cmd_size > 0)
				{
					cmd = realloc(cmd, cmd_size--);
					cmd[cmd_size] = '\0';
					putchar(c);
				}
			break;
			default:
				cmd = realloc(cmd, cmd_size + 2);
				cmd[cmd_size] = c;
				cmd[cmd_size + 1] = '\0';
				cmd_size++;
				putchar(c);
		}
	}

	return 0;
}

static void help()
{
	const command_t *cmds = commands;
	printf("Verfuegbare Befehle:\n");
	while(cmds->cmd != NULL)
	{
		printf("%s\t%s\n", cmds->cmd, cmds->desc);
		cmds++;
	}
}

static void info()
{
	SIS sysinf;
	getSysInfo(&sysinf);
	printf("Uptime:          %lums\n"
			"Speicher:        %lu Bytes\n"
			"Freier Speicher: %lu Bytes\n", sysinf.Uptime, sysinf.physSpeicher, sysinf.physFree);
}

static void ls(const char *arg)
{
	struct dirent **dirents;
	while(isspace(*arg)) arg++;

	if(strlen(arg) == 0)
	{
		printf("No arguments specified\n");
		return;
	}

	int size = scandir(arg, &dirents, NULL, alphasort);
	if(size == -1)
		printf("Could not list %s\n", arg);
	else
	{
		int i;
		for(i = 0; i < size; i++)
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
			printf("%c\t%s\n", type, dirents[i]->d_name);
			free(dirents[i]);
		}
		free(dirents);
	}
}

static void cat(const char *arg)
{
	int c;
	FILE *fp;
	while(isspace(*arg)) arg++;

	if(strlen(arg) == 0)
	{
		printf("No arguments specified\n");
		return;
	}

	fp = fopen(arg, "r");

	if(fp == NULL)
	{
		printf("Could not open %s\n", arg);
		return;
	}

	while((c = getc(fp)) != EOF)
	{
		putchar(c);
	}

	fclose(fp);
	puts("");
}

static void mount(const char *arg)
{
	if(strlen(arg) == 0)
	{
		printf("No arguments specified\n");
		return;
	}

	char *mountpoint = strtok(arg, " ");
	char *device = strtok(NULL, " ");

	if(mountpoint == NULL)
	{
		printf("No mountpoint specified!\n");
		return;
	}
	if(device == NULL)
	{
		printf("No device specified!\n");
		return;
	}

	printf("Trying to mount %s in %s\n", device, mountpoint);
	int status = syscall_mount(mountpoint, device);
	printf("mount return value: %i\n", status);
}

static void unmount(const char *arg)
{
	while(isspace(*arg)) arg++;

	if(strlen(arg) == 0)
	{
		printf("No arguments specified\n");
		return;
	}

	printf("Trying to unmount %s\n", arg);
	int status = syscall_unmount(arg);
	printf("unmount return value: %i\n", status);
}

void command(char *cmd)
{
	const command_t *cmds = commands;
	bool found = false;
	if(cmd == NULL)
		return;
	while(cmds->cmd != NULL)
	{
		size_t cmd_length = strlen(cmds->cmd);
		if(strncmp(cmds->cmd, cmd, cmd_length - 1) == 0)
		{
			cmds->func(cmd + cmd_length);
			found = true;
			break;
		}
		cmds++;
	}
	if(!found)
	{
		printf("Dieser Befehl ist nicht gueltig\nGib 'help' ein um mehr zu erfahren\n");
		putchar('\n');
	}
}
