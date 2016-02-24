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
#include <stdlib.h> //for malloc, free, exit, getenv
#include <unistd.h> //for chdir, fork, exec, pid_t
#include <sys/wait.h> //for witpid
#include <string.h> //for strcmp, strtok
#include <signal.h> //for sigaction 
#include <sys/types.h> //for pid_t
#include <fcntl.h> 

#define MAXSIZE 2048 //used limit the lenght of a command 
#define MAXARG 512 //used to limit number of arguments  
#define DELIM " \t\r\n\a"

void sigHandler(){
   struct sigaction act;
   act.sa_handler = SIG_IGN;
   sigaction(SIGINT, &act, NULL);
}

char *readInput(){
   char *buffer = NULL;
   ssize_t size = MAXSIZE;
   getline(&buffer, &size, stdin);
   return buffer; 
}

char **getArgs(char *input){
   int bufsize = MAXARG;
   char **argsArr = malloc(bufsize * sizeof(char*));
   char *token;
   int i = 0; 
   if(!argsArr){
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
   }
   token = strtok (input, DELIM);
   while (token != NULL){
      argsArr[i] = token;
      i++;
      token = strtok (NULL, DELIM); 
   }
   argsArr[i] = NULL;
   return argsArr;   
}

int execute(char **args, int status){
   //exit command
   if (strcmp("exit", args[0]) == 0){
      exit(EXIT_SUCCESS);
   }
   //cd command
   if (strcmp("cd", args[0]) == 0){
      if (args[1] != NULL){
         chdir(args[1]);
         return 1;
      }
      else{
         chdir(getenv("HOME"));
         return 1;
      }
   }
   //status command
   if (strcmp("status", args[0]) == 0){
      printf("exit value %i\n", status);
      fflush(stdout);
   }
   //
   else { 
      return executeCommand(args);
   }      
}

int executeCommand(char **args){
   int status;
   pid_t cpid, wpid;
   int finput = 0;
   int foutput = 0;
   int bg = 0;

   // while(args[i] != NULL){
   if (args[1] != NULL){  
      if(strcmp(args[1], "<") == 0){
         finput = 1; 
         //change < to NULL; then it will be ignored while execution
         //args[i] = NULL;   
      }
      else if(strcmp(args[1], ">") == 0){
         foutput = 1;
         //args[i] = NULL;
      } 
      //else if(strcmp(args[i], "&") == 0){
         //args[i] = NULL;
        // bg = i;
      //}
      //i++;
   //}
   }
   
   int fd, fd2;
   if (bg == 0){
       if (finput == 0 && foutput == 0){ 
          cpid = fork(); 
          if (cpid == 0){
             if (execvp(args[0], args) == -1){ 
                perror("exec");
                status = 1;
                exit(1);
             }            
          }
          else if (cpid < 0){
             perror("fork()");
             return 1;
          }
          else{
             do{
                wpid = waitpid(cpid, &status, WUNTRACED);
             }while (!WIFEXITED(status) && !WIFSIGNALED(status));
          } 
       }
       //redirect standart input
       else if (finput != 0 && foutput == 0){
          cpid = fork();
  
          if (cpid == 0){
              fd = open(args[finput+1], O_RDONLY);
              if (fd == -1){
                 perror("open");
                 status = 1;
                 exit(EXIT_FAILURE);
              }
              fd2 = dup2(fd,0);
              if (execlp(args[0], args[0], NULL) == -1){ 
                  perror("exec");
                  status = 1;
              }
              exit(EXIT_FAILURE);
                          
          }
          else if (cpid < 0){
             perror("fork()");
             status = 1;
             return status;
          }
          else{
             do{
                wpid = waitpid(cpid, &status, WUNTRACED);
             }while (!WIFEXITED(status) && !WIFSIGNALED(status));
          } 
          close(fd); 
      }

      else {
         cpid = fork();
         
         if (cpid == 0){ 
            fd = open(args[foutput+1], O_WRONLY|O_CREAT|O_TRUNC,0644);
            if (fd == -1){
               perror("open");
               status = 1;
               exit(EXIT_FAILURE);
             }
             fd2 = dup2(fd,1);
          
             if (execlp(args[0], args[0], NULL) == -1){ 
                perror("exec");
                status = 1;
             }
                exit(EXIT_FAILURE);
          }
          else if (cpid < 0){
             perror("fork()");
             status = 1;
             return status;
          }
          else{
             do{
                wpid = waitpid(cpid, &status, WUNTRACED);
             }while (!WIFEXITED(status) && !WIFSIGNALED(status));
          } 
          close(fd);  
       }
      
   }
   return 1; 
}

int main(){
   char *input;
   char **args;
   int status = 0;
   char comment = '#';
   char blank = ' ';

   sigHandler();  

   do{
      printf(": ");
      fflush(stdout);
      input = readInput();
      //if an user entered #, it is a comment, ignore
      //if an user entered space ignore the line
      if(input[0] == comment || input[0] == blank){
         continue;
      }
      args = getArgs(input);
      status = execute(args, status);

      free(input);
      free(args);
   }while(1);
   return EXIT_SUCCESS;
}               
