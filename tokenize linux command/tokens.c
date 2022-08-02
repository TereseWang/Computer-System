#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "svec.c"
#include <string.h>

//push the text between index ii and jj to the input list 
void
read_args(long ii, long jj, svec* xs, const char* text)
{
    size_t args_size = ii - jj;
    char* result = malloc((args_size + 1) * sizeof(char));
    memcpy(result, &text[jj], args_size);
    result[ii - jj] = 0;
    svec_push_back(xs, result);
    free(result);
}

svec*
tokenize(const char* text)
{
    svec* xs = make_svec();
    long ii = 0;//current index 
    long nn = strlen(text);
    long jj = 0; //current place for last non operator characters
    while(ii < nn) {
        //if current characte is a space 
        if(isspace(text[ii])) {
            //if there are non-op characters before space
            //push that part of text to the list
            if(ii != jj) {
                read_args(ii, jj, xs, text);
                jj = ii + 1;
                ++ii;
            }
            //else ignore 
            else {
                ii++;
                jj++;
            }
        }
        //if currect character is an operator of <,> or ;
        else if (text[ii] == '<' || text[ii] == '>' || text[ii] == ';'){
            //if there are non-op non-space characters before operator
            //meaning between index ii and jj of the text 
            //push all the characters between ii and jj to the list, 
            //then makes ii and jj equal
            if(ii != jj) {
                read_args(ii, jj, xs, text);
            }
            //else, just push the operator to the list
            read_args(ii + 1, ii, xs, text);
            jj = ii + 1;
            ++ii;
        }
        //if current character is | or &
        else if (text[ii] == '|' || text[ii] == '&') {
            //if there are non-op non space characters before operator
            //push all the characters between ii and jj to the list
            if (ii != jj) {
                read_args(ii, jj, xs, text);
            }
            //if the next character is equal to the current one, meaning 
            //&& or ||, append the two character and push to the list 
            if(ii + 1 < nn && text[ii + 1] == text[ii]) {
                read_args(ii+2, ii, xs, text);
                jj = ii + 2;
                ii = ii + 2;
            }
            //else just push one character 
            else {
                read_args(ii+1, ii, xs, text);
                jj = ii + 1;
                ++ii;
            }
        }
        else {
            ++ii;
        }
    }
    return xs;
}

int 
main(int argc, char* argv[]) {
    char line[100];
    while(1) {
        printf("tokens$ ");
        fflush(stdout);
        char* curr = fgets(line, 96, stdin);
        if(!curr) {
            break;
        }
        svec* xs = tokenize(line);
        for(int ii = xs->size - 1; ii >= 0; --ii) {
            printf("%s\n", svec_get(xs, ii));
        }
        free_svec(xs);
    }
    return 0;
}



