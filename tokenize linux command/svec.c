/* This file is lecture notes from CS 3650, Fall 2018 */
/* Author: Nat Tuck */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "svec.h"

svec*
make_svec()
{
    svec* sv = malloc(sizeof(svec));
    sv->data = malloc(2 * sizeof(char*));
    sv->size = 0;
    sv->cap = 2;
    return sv;
}

void
free_svec(svec* sv)
{
	for (int i = 0; i < sv->size; i++) {
		free(sv->data[i]);
	}
	free(sv->data);
	free(sv);
}

char*
svec_get(svec* sv, int ii)
{
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

void
svec_put(svec* sv, int ii, char* item)
{
    assert(ii >= 0 && ii < sv->size);
    sv->data[ii] = strdup(item);
}

void
svec_push_back(svec* sv, char* item)
{
    int ii = sv->size;
    if(ii == sv->cap) {
	    sv->cap = sv->cap * 2;
	    sv->data = (char**) realloc(sv->data, sv->cap*(sizeof(char*)));
    }
    sv->size = ii + 1;
    svec_put(sv, ii, item);
}

void
svec_swap(svec* sv, int ii, int jj)
{
    char* temp = sv->data[ii];
    char* temp1 = sv->data[jj];
    sv->data[ii] = temp1;
    sv->data[jj] = temp;
}
