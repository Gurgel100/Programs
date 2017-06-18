/*
 * main.c
 *
 *  Created on: 28.10.2014
 *      Author: pascal
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	for(int i = 1; i < argc; i++)
	{
		if(i == 1)
			printf("%s", argv[i]);
		else
			printf(" %s", argv[i]);
	}
	putchar('\n');
	return EXIT_SUCCESS;
}
