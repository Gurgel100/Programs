/*
 * suggestions.h
 *
 *  Created on: 01.05.2017
 *      Author: pascal
 */

#ifndef SUGGESTIONS_H_
#define SUGGESTIONS_H_

#include <stddef.h>

typedef struct{
	const char *suggestion;
	size_t prefix_len;
	enum{
		SUG_PATH, SUG_FILE
	}type;
}suggestion_entry_t;

typedef struct{
	size_t num;
	char *shortest_suggestion;
	suggestion_entry_t *suggestions;
}suggestions_t;

#endif /* SUGGESTIONS_H_ */
