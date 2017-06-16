/*
 * history.h
 *
 *  Created on: 16.06.2017
 *      Author: pascal
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include <stddef.h>

typedef struct history history_t;

history_t *history_init();

void history_add(history_t *history, const char *cmd);
char *history_get(history_t *history, size_t i);

#endif /* HISTORY_H_ */
