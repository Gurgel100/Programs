/*
 * main.c
 *
 *  Created on: 28.10.2014
 *      Author: pascal
 */

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <userlib.h>
#include <math.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <syscall.h>
#include <assert.h>
#include "suggestions.h"

typedef struct{
	const char *cmd;
	const char *desc;
	void (*func)(const char**, size_t);
}command_t;

void command(char *cmd);
double c;

static void help();
static void info();
static void ls(const char**, size_t);
static void cat(const char**, size_t);
static void mount(const char**, size_t);
static void unmount(const char**, size_t);
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
	suggestions_t suggestions;
	bool last_character_was_tab = false;
	size_t cmd_size = 0;
	char *cmd = calloc(cmd_size + 1, 1);
	printf("Willkommen bei YourOS V0.1\n");
	putchar('\n');
	putchar('>');
	//Einfache ein-/ausgabe
	while(true)
	{
		char c = getchar();
		bool last_tab = last_character_was_tab;
		last_character_was_tab = false;
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
			case '\t':
				get_suggestions(cmd, &suggestions);
				if(suggestions.num > 1)
				{
					if(suggestions.shortest_suggestion != NULL)
					{
						//Add suggestion prefix to command
						size_t suggestion_len = strlen(suggestions.shortest_suggestion);
						cmd = realloc(cmd, cmd_size + suggestion_len + 1);
						cmd_size += suggestion_len;
						strcat(cmd, suggestions.suggestions[0].suggestion);
						//Print suggestion prefix
						printf("%s", suggestions.shortest_suggestion);
					}
					else if(last_tab)
					{
						puts("");
						for(size_t i = 0; i < suggestions.num; i++)
							printf("%s%s\t", &cmd[cmd_size - suggestions.suggestions[i].prefix_len], suggestions.suggestions[i].suggestion);
						printf("\n>%s", cmd);
					}
				}
				else if(suggestions.num == 1)
				{
					//Add suggestion to command
					char *extra_sug;
					switch(suggestions.suggestions[0].type) {
						case SUG_PATH:
							//Add '/'
							extra_sug = "/";
						break;
						default:
							extra_sug = "";
					}
					size_t suggestion_len = strlen(suggestions.suggestions[0].suggestion) + strlen(extra_sug);
					cmd = realloc(cmd, cmd_size + suggestion_len + 1);
					cmd_size += suggestion_len;
					strcat(cmd, suggestions.suggestions[0].suggestion);
					strcat(cmd, extra_sug);
					//Print suggestion
					printf("%s%s", suggestions.suggestions[0].suggestion, extra_sug);
				}
				free_suggestions(&suggestions);
				last_character_was_tab = true;
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

static void ls(const char **args, size_t arg_count)
{
	struct dirent **dirents;

	if(arg_count == 0)
	{
		printf("No arguments specified\n");
		return;
	}
	
	const char *arg = args[0];

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

static void cat(const char **args, size_t arg_count)
{
	int c;
	FILE *fp;

	if(arg_count == 0)
	{
		printf("No arguments specified\n");
		return;
	}

	for(size_t i = 0; i < arg_count; i++)
	{
		const char *arg = args[i];
		while(isspace(*arg)) arg++;

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
}

static void mount(const char **args, size_t arg_count)
{
	if(arg_count < 2)
	{
		printf("Usage: mount [device] [mountpoint]\n");
		return;
	}

	const char *device = args[0];
	const char *mountpoint = args[1];

	printf("Trying to mount %s in %s\n", device, mountpoint);
	int status = syscall_mount(mountpoint, device);
	printf("mount return value: %i\n", status);
}

static void unmount(const char **args, size_t arg_count)
{
	if(arg_count == 0)
	{
		printf("Usage: umount [mountpoint]\n");
		return;
	}

	printf("Trying to unmount %s\n", args[0]);
	int status = syscall_unmount(args[0]);
	printf("unmount return value: %i\n", status);
}

static const char *get_end_of_env(const char *env)
{
	if(isdigit(*env))
		return env + 1;
	while(isalnum(*env) || *env == '_') env++;
	return env;
}

static bool add_token(const char *token, char ***tokens, size_t *token_count)
{
	if(strlen(token) > 0)
	{
		char **new_tokens = realloc(*tokens, (*token_count + 1) * sizeof(char**));
		if(new_tokens == NULL)
		{
			printf("An error occured during parsing: out of memory!\n");
			return false;
		}
		*tokens = new_tokens;
		(*tokens)[*token_count] = strdup(token);
		if((*tokens)[*token_count] == NULL)
		{
			printf("An error occured during parsing: out of memory!\n");
			return false;
		}
		(*token_count)++;
	}
	return true;
}

static size_t tokenize(const char *cmd, char ***tokens)
{
	size_t token_count = 0;
	bool escape = false;	//indicates that '\' was the previous character
	char quotes = '\0';
	size_t current_token_len = strlen(cmd);
	char *current_token = malloc(current_token_len + 1);
	size_t current_index = 0;
	*tokens = NULL;

	while(*cmd != '\0')
	{
		switch(*cmd)
		{
			case '\\':
				if(escape)
				{
					current_token[current_index++] = '\\';
					escape = false;
				}
				else
					escape = true;
			break;
			case ' ':
				if(!escape && quotes == '\0')
				{
					current_token[current_index] = '\0';
					if(!add_token(current_token, tokens, &token_count))
						goto error;
					current_index = 0;
				}
				else
				{
					current_token[current_index++] = ' ';
					escape = false;
				}
			break;
			case '\"':
				if(escape)
				{
					current_token[current_index++] = '\'';
					escape = false;
				}
				else
				{
					switch(quotes)
					{
						case '\0':
							quotes = '\"';
						break;
						case '\"':
							quotes = '\0';
						break;
						case '\'':
							current_token[current_index++] = '\"';
						break;
					}
				}
			break;
			case '\'':
				if(escape)
				{
					current_token[current_index++] = '\'';
					escape = false;
				}
				else
				{
					switch(quotes)
					{
						case '\0':
							quotes = '\'';
						break;
						case '\'':
							quotes = '\0';
						break;
						case '\"':
							current_token[current_index++] = '\'';
						break;
					}
				}
			break;
			case '$':
				if(!escape)
				{
					char *env;
					const char *env_end;
					if(*(cmd + 1) == '?')
					{
						env_end = cmd + 2;
						int size = snprintf(NULL, 0, "%i", last_exit_code) + 1;
						env = __builtin_alloca(size);
						snprintf(env, size, "%i", last_exit_code);
					}
					else
					{
						env_end = get_end_of_env(cmd + 1);
						char env_name[env_end - (cmd + 1) + 1];
						strncpy(env_name, cmd + 1, env_end - (cmd + 1));
						env_name[env_end - (cmd + 1)] = '\0';
						env = getenv(env_name);
					}
					if(env != NULL)
					{
						size_t env_len = strlen(env);
						size_t max_len = current_index + env_len + strlen(cmd + 1);
						if(max_len > current_token_len)
						{
							char *new_token = realloc(current_token, max_len);
							if(new_token == NULL)
							{
								fputs("Could not allocate memory for inserting environment variable\n", stderr);
								goto error;
							}
							current_token = new_token;
							current_token_len = max_len;
						}
						memcpy(&current_token[current_index], env, env_len);
						current_index += env_len;
					}
					//subtract 1 because we add 1 at the end of the loop
					cmd = env_end - 1;
					break;
				}
			break;
			default:
				current_token[current_index++] = *cmd;
				escape = false;
			break;
		}
		cmd++;
	}

	current_token[current_index] = '\0';
	if(!add_token(current_token, tokens, &token_count))
		goto error;

	free(current_token);

	return token_count;

error:
	for(size_t i = 0; i < token_count; i++)
		free((*tokens)[i]);
	free(*tokens);
	free(current_token);
	return 0;
}

void command(char *cmd)
{
	char **tokens;
	size_t token_count;
	const command_t *cmds = commands;
	bool found = false;
	if(cmd == NULL || strlen(cmd) == 0)
		return;

	token_count = tokenize(cmd, &tokens);

	if(token_count > 0)
	{
		while(cmds->cmd != NULL)
		{
			if(strcmp(cmds->cmd, tokens[0]) == 0)
			{
				cmds->func((const char**)&tokens[1], token_count - 1);
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
		for(size_t i = 0; i < token_count; i++)
			free(tokens[i]);
		free(tokens);
	}
}
