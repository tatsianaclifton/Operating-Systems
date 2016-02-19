/*************************************************************
 * CS344 Program 3 - smallsh                                 *
 * Author: Tatsiana Clifton                                  *
 * Date: 2/17/2016                                           *
 * Description: The program is a shell that works like the   *
 *              bash shell, prompting for a command line and *
 *              running commands. It allows for the          *
 *              rederection of standart input and standard   *
 *              output. It supports both foreground and      *
 *              background processes. It supports three built*
 *              in commands: exit, cd, and status.           *
 *              It suppotrs comments, which are lines        *
 *              beginning with the # character.              *
 * Usage:                                                    *
 * command [arg1 arg2 ...] [< input_file] [> output_file] [&]*
 * Sources: man pages man7.org                               *
 *          brennan.io/2015/01/16/write-a-shell-in-c         *  
 *************************************************************/ 

#include <stdio.h> //for fprintf, stderr, perror
#include <stdlib.h> //for malloc, free, exit
#include <unistd.h> //for chdir, fork, exec, pid_t
#include <sys/wait.h> //for witpid
#include <string.h> //for strcmp, strtok

#define MAXSIZE 2048 //used limit the lenght of a command 
#define MAXARG 512 //used to limit number of arguments  

char *readInput(){
   char *buffer = NULL;
   ssize_t size = MAXSIZE;
   getline(&buffer, &size, stdin);
   return buffer; 
}

char **getArgs(char *input){
   char **argsArr = malloc(MAXARG * sizeof(char*));
   char *token;
   int i = 0; 
   if(!token){
      fprintf(stderr, "allocation error\n");
      exit(1);
   }
   token = strtok (input, "\n");
   while (token != NULL){
      argsArr[i] = token;
      i++;
      token = strtok (NULL, "\n"); 
   }
   argsArr[i] = NULL;
   return argsArr;   
}

void execute(char **args){
   if (strcmp("exit", args[0]) == 0){
      exit(0);
   }
   if (strcmp("cd", args[0]) == 0){
      if(args[1] != NULL){
         if (chdir(args[1]) != 0){
            perror("error");
         }
      }
      else{
         chdir(getenv("HOME"));
      }
   }
   else{
      printf("NOT\n");
   }      
}

int main(){
   char *input;
   char **args;
   char comment = '#';
   char blank = ' ';

   while(1){
      printf(": ");
      fflush(stdout);
      input = readInput();
      //if an user entered #, it is a comment, ignore
      //if an user entered space ignore the line
      if(input[0] == comment || input[0] == blank){
         continue;
      }
      args = getArgs(input);
      execute(args);
      
      free(input);
      free(args);
   }
   return 0;
}               
