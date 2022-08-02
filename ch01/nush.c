#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "svec.h"
#include "tokens.h"
int cpid;
int status;
void execute(svec* command);

//create a subshell to run the command inside the parentheses
void
parentheses_helper(svec* command, int start, int end) {
  //command inside the parentheses
  svec* mid = svec_sub(command, start + 1, end);
  //parent wait for child to finish
  if(cpid = fork()) {
    waitpid(cpid, &status, 0);
    free(mid);
  }
  //in child, excute the given command
  else {
    execute(mid);
  }
}

//read from the given file
void
read_helper(svec* command, int operator) {
  //split the command into to part, the left will be other command
  svec* left = svec_sub(command, 0, operator);
  //the right will be the file need to input from
  svec* right = svec_sub(command, operator + 1, command->size);
  if(cpid = fork()) {
    waitpid(cpid, &status, 0);
    free_svec(left);
    free_svec(right);
  }
  else {
    //From Nat Tuck CS3650 Lecture 11 redir.c
    int fd = open(svec_get(right, 0), O_CREAT | O_RDONLY, 0644);
    close(0);
    dup(fd);
    execute(left);
    close(fd);
  }
}

//output to the given file
void
write_helper(svec* command, int operator) {
  //split the command into to part, the left will be other command
  svec* left = svec_sub(command, 0, operator);
  //the right will be the file need to output to
  svec* right = svec_sub(command, operator + 1, command->size);
  if(cpid = fork()) {
    waitpid(cpid, &status, 0);
    free_svec(left);
    free_svec(right);
  }
  else {
    //From Nat Tuck CS3650 Lecture 11 redir.c
    int fd = open(svec_get(right, 0), O_CREAT | O_APPEND | O_WRONLY, 0644);
    close(1);
    dup(fd);
    execute(left);
    close(fd);
  }
}

//split the command into two part and run them together
void semicolon_helper(svec* command, int semicolon_index) {
  svec* left = svec_sub(command, 0, semicolon_index);
  svec* right = svec_sub(command, semicolon_index + 1, command->size);
  if(cpid = fork()) {
    waitpid(cpid, &status, 0);
    free_svec(left);
    free_svec(right);
  }
  else {
    execute(left);
    execute(right);
  }
}

void
pipe_helper(svec* command, int index) {
  svec* left = svec_sub(command, 0, index);
  svec* right = svec_sub(command, index + 1, command->size);
  //reference for usage of pipe
  //https://www.geeksforgeeks.org/pipe-system-call/
  int p[2];
  if(pipe(p) < 0) {
    exit(1);
  }
  else {
    if(cpid = fork()) {
      close(0);
      dup(p[0]);
      close(p[1]);
      execute(right);
    }
    else {
      close(1);
      dup(p[1]);
      close(p[0]);
      execute(left);
      exit(0);
    }
  }
}

//run the command in background
void
background_helper(svec* command, int index){
  if(cpid = fork()) {
    //waitpid(cpid, &status, 0);
  }
  else {
    //waitpid(cpid, &status, 0);
    svec* left = svec_sub(command, 0, index + 1);
    char* command_to_execute[left->size + 1];
    for (int i = 0; i < left->size; i++) {
      command_to_execute[i] = svec_get(left, i);
    }
    command_to_execute[left->size] = 0;
    execvp(command_to_execute[0], command_to_execute);
    _Exit(0);
  }
}

//evaluate the left part command, if command passed, run right part command
void
and_helper(svec* command, int index) {
  svec* left = svec_sub(command, 0, index);
  svec* right = svec_sub(command, index + 1, command->size);
  //parent wait for child to complete, if complete successfully run right part
  if(cpid = fork()) {
    int current_status;
    waitpid(cpid, &current_status, 0);
    if (current_status == 0) {
      execute(right);
    }
  }
  //child
  else {
    //append all the items in the command list of left part, if left parts
    //runs successfully exit back to parent
    char* command_to_execute[left->size + 1];
    for (int i = 0; i < left->size; i++) {
      command_to_execute[i] = svec_get(left, i);
    }
    command_to_execute[left->size] = 0;
    execvp(command_to_execute[0], command_to_execute);
    _Exit(0);
  }
  free_svec(left);
  free_svec(right);
}

//evaluate the left part command, if command failed, run right part command
void
or_helper(svec* command, int index) {
  svec* left = svec_sub(command, 0, index);
  svec* right = svec_sub(command, index + 1, command->size);
  //parent wait for left part to run, if run failed, run right part
  if(cpid = fork()) {
    int current_status;
    waitpid(cpid, &current_status, 0);
    if (current_status != 0) {
      execute(right);
    }
  }
  //child run left part of the command
  else {
    char* command_to_execute[left->size + 1];
    for (int i = 0; i < left->size; i++) {
      command_to_execute[i] = svec_get(left, i);
    }
    command_to_execute[left->size] = 0;
    execvp(command_to_execute[0], command_to_execute);
    _Exit(0);
  }
  free_svec(left);
  free_svec(right);
}

//execute the command by splitting up them into pieces of special operators
//executable commands
void
execute(svec* command){
  //creating subshells
  if(svec_contain(command, "(") != -1
          && svec_contain(command, ")") != -1){
    int start = svec_contain(command, "(");
    parentheses_helper(command, start, command->size - 1);
    if(command->size > 2) {
      execute(svec_sub(command, 2, command->size));
    }
  }
  //input file
  else if(svec_contain(command, "<") != -1) {
    int operator_index = svec_contain(command, "<");
    read_helper(command, operator_index);
  }
  //output file
  else if(svec_contain(command, ">") != -1){
    int operator_index = svec_contain(command, ">");
    write_helper(command, operator_index);
  }
  //separate the command
  else if(svec_contain(command, ";") != -1){
    int semicolon_index = svec_contain(command, ";");
    semicolon_helper(command, semicolon_index);
  }
  //pipe command
  else if(svec_contain(command, "|") != -1){
    int index = svec_contain(command, "|");
    pipe_helper(command, index);
  }
  //background command
  else if(svec_contain(command, "&") != -1){
    int index = svec_contain(command, "&");
    background_helper(command, index);
  }
  //and logical command
  else if(svec_contain(command, "&&") != -1){
    int index = svec_contain(command, "&&");
    and_helper(command, index);
  }
  //or logical command
  else if(svec_contain(command, "||") != -1){
    int index = svec_contain(command, "||");
    or_helper(command, index);
  }
  //cd dir command
  else if(strcmp(svec_get(command, 0), "cd") == 0) {
    chdir(svec_get(command, 1));
    if(command->size > 2) {
      execute(svec_sub(command, 2, command->size));
    }
  }
  //exit command
  else if(strcmp(svec_get(command, 0), "exit") == 0) {
    exit(0);
  }
  //other pre installed command in shell
  else {
    //parent
    if((cpid = fork())) {
      waitpid(cpid, &status, 0);
    }
    //child
    else {
      //append all items in the command list into one string
      char* command_to_execute[command->size + 1];
      for (int i = 0; i < command->size; i++) {
        command_to_execute[i] = svec_get(command, i);
      }
      command_to_execute[command->size] = 0;
      //execute the command
      execvp(command_to_execute[0], command_to_execute);
      exit(0);
    }
  }
}

int
main(int argc, char* argv[])
{
    char cmd[256];
    svec* command;
    //run interactive shell
    if (argc == 1) {
      //From Nat Tuck CS3650 Lecture 10 calc.c
      //https://github.com/NatTuck/scratch-2021-01/blob/master/notes-3650/10-calc-fork/calc/calc.c
      while(1){
        printf("nush$ ");
        fflush(stdout);
        char* line = fgets(cmd,256, stdin);
        if (!line) {
          break;
        }
        command = tokenize(cmd);
        execute(command);
        free_svec(command);
      }
    }
    //read file
    else {
      //From Nat Tuck CS3650 Lecture 10  file-io/cstd.c
      //https://github.com/NatTuck/scratch-2021-01/blob/master/notes-3650/10-calc-fork/file-io/cstd.c
        FILE* file = fopen(argv[1], "r");
        while (fgets(cmd, 256, file)) {
          command = tokenize(cmd);
          execute(command);
          free_svec(command);
        }
        fclose(file);
    }
    return 0;
}
