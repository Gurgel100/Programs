/*
 * history.c
 *
 *  Created on: 16.06.2017
 *      Author: pascal
 */

#include "history.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct history{
	const char **history;
	size_t count;
};

history_t *history_init()
{
	return calloc(1, sizeof(history_t));
}

void history_add(history_t *history, const char *cmd)
{
	const char **new_ptr = realloc(history->history, (history->count + 1) * sizeof(char*));
	if(new_ptr != NULL)
	{
		history->history = new_ptr;
		history->history[history->count++] = strdup(cmd);
	}
}

char *history_get(history_t *history, size_t back)
{
	if(back + 1 > history->count)
		return NULL;

	return strdup(history->history[history->count - back - 1]);
}
