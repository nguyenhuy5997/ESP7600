/*
 * string_parse.c
 *
 *  Created on: 27 Jun 2022
 *      Author: nguyenphuonglinh
 */
#include "string_parse.h"
char *getSubStrig(char *source, char *start, char *end)
{
	int j = 0, k = 0, index_start = 0, index_end = 0;
	for(int i = 0; i < strlen(source); i++)
	{
		if (source[i] == start[j])
		{
			j++;
			if (j == strlen(start))
			{
			    index_start = i + 1;
			}
		}
		else j = 0;
		if( source[i] == end[k])
		{
			k++;
			if (k == strlen(end))
			{
				index_end = i - strlen(end) + 1;
				break;
			}
		}
		else
		{
		    k = 0;
		}
	}
	char * sub_buf = malloc(sizeof(char) * strlen(source));
	memcpy(sub_buf, source + index_start, index_end - index_start);
	return sub_buf;
}


