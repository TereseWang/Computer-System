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

//return the index of the given string if the list contains this string
//else return -1
int svec_contain(svec* sv, char* str)
{
  for(int i = 0; i < sv->size; i++) {
    if(strcmp(svec_get(sv, i), str) == 0) {
      return i;
      break;
    }
  }
  return -1;
}

void
svec_print(svec* sv) {
	for (int i = 0; i < sv->size; ++i) {
		printf("%s\n", svec_get(sv, i));
	}
}

svec*
svec_sub(svec* sv, int ii, int jj) {
  svec* result = make_svec();
  for (int i = ii; i < jj; i++) {
    svec_push_back(result, svec_get(sv, i));
  }
  return result;
}
