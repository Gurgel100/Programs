/*
 * suggestions.c
 *
 *  Created on: 01.05.2017
 *      Author: pascal
 */

#include "suggestions.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static char *sel_base_prefix;
static size_t sel_base_prefix_len;

static int suggestions_selector(const struct dirent *dirent)
{
	return !strncmp(sel_base_prefix, dirent->d_name, sel_base_prefix_len);
}

static size_t search_path_suggestions(const char *input, suggestions_t *suggestions, size_t *longest_prefix_len)
{
	assert(input != NULL);
	assert(suggestions != NULL);
	assert(longest_prefix_len != NULL);

	const char *base_prefix;
	const char *actual_path;
	struct dirent **dirents = NULL;
	char *path = strdup(input);
	if(path == NULL)
		return 0;
	char *last_slash = strrchr(path, '/');
	if(last_slash != NULL)
	{
		if(last_slash - path > 0)
		{
			*last_slash = '\0';
			base_prefix = last_slash + 1;
		}
		else
		{
			size_t path_len = strlen(path);
			path = realloc(path, path_len + 2);
			base_prefix = memmove(path + 2, path + 1, path_len - 1);
			path[1] = '\0';
		}
		actual_path = path;
	}
	else
	{
		//TODO: use PATH environment variable when available
		actual_path = "/";
		base_prefix = path;
	}

	size_t base_prefix_len = strlen(base_prefix);

	//Parameter for suggestion_selector
	sel_base_prefix = (char*)base_prefix;
	sel_base_prefix_len = base_prefix_len;
	int size = scandir(actual_path, &dirents, suggestions_selector, alphasort);
	//We don't need path anymore
	free(path);

	size_t start_index = suggestions->num;
	if(size > 0)
	{
		suggestion_entry_t *prev_mem = suggestions->suggestions;
		suggestions->suggestions = realloc(prev_mem, (suggestions->num + size) * sizeof(suggestion_entry_t));
		if(suggestions->suggestions == NULL)
		{
			suggestions->suggestions = prev_mem;
		}
		else
		{
			size_t sug_index = suggestions->num;
			size_t shortest_suggestion_len = dirents[0]->d_reclen - sizeof(struct dirent) - 1 - base_prefix_len;
			for(size_t i = 0; i < (unsigned)size; i++)
			{
				assert(dirents[i]->d_reclen - sizeof(struct dirent) - 1 >= base_prefix_len);
				const char *dirname = strdup(dirents[i]->d_name + base_prefix_len);
				d_type_t type = dirents[i]->d_type;
				free(dirents[i]);
				if(dirname == NULL)
					continue;

				suggestion_entry_t *entry = &suggestions->suggestions[sug_index++];
				entry->suggestion = dirname;
				entry->prefix_len = base_prefix_len;
				if(type == DT_DIR)
					entry->type = SUG_PATH;
				else
					entry->type = SUG_FILE;
			}
			suggestions->num += sug_index - suggestions->num;

			//Search for longest common prefix
			for(size_t i = 0; i < suggestions->num; i++)
			{
				for(size_t j = 0; j < shortest_suggestion_len; j++)
				{
					//Because the suggestions are ordered lexically we know that the first entry is the shortest
					if(suggestions->suggestions[0].suggestion[j] != suggestions->suggestions[i].suggestion[j])
					{
						shortest_suggestion_len = j;
						break;
					}
				}
			}
			*longest_prefix_len = shortest_suggestion_len;
		}
	}

	if(size >= 0)
		free(dirents);

	return start_index;
}

void get_suggestions(const char *input, suggestions_t *suggestions)
{
	const char *last_space = strrchr(input, ' ');
	const char *last_param = last_space != NULL ? last_space + 1 : input;
	suggestions->num = 0;
	suggestions->shortest_suggestion = NULL;
	suggestions->suggestions = NULL;

	size_t path_prefix_len = 0;
	size_t path_start_index = search_path_suggestions(last_param, suggestions, &path_prefix_len);

	if(path_prefix_len > 0)
	{
		suggestions->shortest_suggestion = malloc(path_prefix_len + 1);
		if(suggestions->shortest_suggestion != NULL)
		{
			strncpy(suggestions->shortest_suggestion, suggestions->suggestions[path_start_index].suggestion, path_prefix_len);
			suggestions->shortest_suggestion[path_prefix_len] = '\0';
		}
	}
}

void free_suggestions(suggestions_t *suggestions)
{
	for(size_t i = 0; i < suggestions->num; i++)
		free((void*)suggestions->suggestions[i].suggestion);
	free(suggestions->shortest_suggestion);
	free(suggestions->suggestions);
}
