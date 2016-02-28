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
#define DELIM " \n\t\r\a"

int signo;

void sigHandler(){
   int status;
   int signo;
   pid_t cpid;
   char msg[100]; 
   char sigNum[5];
   char statusNum[3];
   //for all child processes
   while((cpid = waitpid(-1, &status, WNOHANG)) > 0){
      char pid[15];
      snprintf(pid, 15, "%i", cpid);

      strncpy(msg, "background pid ", 100);
      strcat(msg, pid);
      strcat(msg, " is done:  ");
      //if a child procces is terminated by signal
      if (WIFSIGNALED(status)){
          signo = WTERMSIG(status);
          snprintf(sigNum, 5, "%i", signo);
          strcat(msg, "terminated by signal ");
          strcat(msg, sigNum);
          strcat(msg, "\n");
          write(1, msg, 100); 
      }
      //if a child proccess is finished normal
      else{
         status = WEXITSTATUS(status);
         snprintf(statusNum, 3, "%i", status);
         strcat(msg, "exit value ");
         strcat(msg, statusNum);
         strcat(msg, "\n");
         write(1, msg, 100);    
      }
      continue;
   } 
}

char *readInput(){
   ssize_t size = MAXSIZE;
   char *buffer = NULL;
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
      //printf("%s\n", argsArr[i]);
      //fflush(stdout); 
      i++;
      token = strtok (NULL, DELIM); 
   }
   argsArr[i] = NULL;
   //printf("%s\n", argsArr[i]);
   //fflush(stdout); 
  
   return argsArr;   
}

int execute(char **args, int status){
   if (args[0] == NULL){
      return 1; 
   }
   //exit command
   if (strcmp("exit", args[0]) == 0){
      //stop = 1;
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
      if (status == 2){
         printf("terminated by signal %i\n", signo);
         fflush(stdout);
         status = 0; 
      }
      else{
         printf("exit value %i\n", status);
         fflush(stdout);
         status = 0;
      }   
   }
   //
   else { 
      return executeCommand(args);
   }      
}

int executeCommand(char **args){
   int status = 0;
   int statusReturn = 0;
   pid_t cpid, wpid;
   int finput = 0;
   int foutput = 0;
   int bg = 0;

   if (args[1] != NULL){  
      if(strcmp(args[1], "<") == 0){
         finput = 1;          
      }
      else if(strcmp(args[1], ">") == 0){
         foutput = 1;
      } 
   }

   int i;
   for (i = 0; args[i] != NULL; i++){
      if ((strncmp(args[i], "&", 1)) == 0){
          bg = 1;
          args[i] = NULL;
      }
   }
   
   int fd, fd2;
   if (bg == 0){

       struct sigaction act;
       act.sa_flags = 0;
       act.sa_handler = SIG_DFL;
       sigaction(SIGINT, &act, NULL);

       if (finput == 0 && foutput == 0){ 
          cpid = fork(); 
          if (cpid == 0){
             if (execvp(args[0], args) == -1){ 
                perror("exec");
                statusReturn = 1;
                exit(EXIT_FAILURE);
             }            
          }
          else if (cpid < 0){
             perror("fork()");
             return 1;
          }
          else{
             //handler for parent proccess to ignore CTRL-C
             struct sigaction act;
             act.sa_handler = SIG_DFL;
             act.sa_handler = SIG_IGN;
             sigaction(SIGINT, &act, NULL);   
             /*do{
                //wait for chil proccess to finish
                wpid = waitpid(cpid, &status, WUNTRACED);
             }while (!WIFEXITED(status) && !WIFSIGNALED(status));*/

             waitpid(cpid, &status, 0);
             statusReturn = WEXITSTATUS(status);

             //if terminated by signal show termination message
             if(WIFSIGNALED(status)){
                signo = WTERMSIG(status);
                printf("terminated by signal %i\n", signo);
                statusReturn = 2;
             }
             //status = 1; 
          } 
       }
       //redirect standart input
       else if (finput != 0 && foutput == 0){
          cpid = fork();
  
          if (cpid == 0){
              fd = open(args[finput+1], O_RDONLY);
              if (fd == -1){
                 perror("open");
                 statusReturn = 1;
                 exit(EXIT_FAILURE);
              }
              fd2 = dup2(fd,0);
              if (execlp(args[0], args[0], NULL) == -1){ 
                  perror("exec");
                  statusReturn = 1;
                  exit(EXIT_FAILURE);  
              }
          }
          else if (cpid < 0){
             perror("fork()");
             statusReturn = 1;
             return statusReturn;
          }
          else{
             //handler for parent proccess to ignore CTRL-C
             /*struct sigaction act;
             act.sa_handler = SIG_DFL;
             act.sa_handler = SIG_IGN;
             sigaction(SIGINT, &act, NULL);*/   
             do{
                //wait for chil proccess to finish
                wpid = waitpid(cpid, &status, WUNTRACED);
             }while (!WIFEXITED(status) && !WIFSIGNALED(status));

             //if terminated by signal show termination message
            /* if(WIFSIGNALED(status)){
                int signo = WTERMSIG(status);
                printf("terminated by signal %i\n", signo);
             }*/
          } 
          close(fd); 
      }
      
      else {
         cpid = fork();
         
         if (cpid == 0){ 
            fd = open(args[foutput+1], O_WRONLY|O_CREAT|O_TRUNC,0644);
            if (fd == -1){
               perror("open");
               statusReturn = 1;
               exit(EXIT_FAILURE);
             }
             fd2 = dup2(fd,1);
          
             if (execlp(args[0], args[0], NULL) == -1){ 
                perror("exec");
                statusReturn = 1;
                //exit(EXIT_FAILURE);
             }
             exit(EXIT_FAILURE);
          }
          else if (cpid < 0){
             perror("fork()");
             statusReturn = 1;
             return statusReturn;
          }
          else{
             //handler for parent proccess to ignore CTRL-C
             /*struct sigaction act;
             act.sa_handler = SIG_DFL;
             act.sa_handler = SIG_IGN;
             sigaction(SIGINT, &act, NULL); */  
             do{
                //wait for chil proccess to finish
                wpid = waitpid(cpid, &status, WUNTRACED);
             }while (!WIFEXITED(status) && !WIFSIGNALED(status));

             //if terminated by signal show termination message
             /*if(WIFSIGNALED(status)){
                int signo = WTERMSIG(status);
                printf("terminated by signal %i\n", signo);
             } */
          } 
          close(fd);  
       }
   }
   else if (bg != 0){
      cpid = fork();

      if (finput == 1){
         fd = open(args[finput+1], O_RDONLY, 0644);
      }

      if (foutput == 1){
         fd = open(args[foutput+1], O_WRONLY|O_TRUNC|O_CREAT, 0644);
      }

      else{
         fd = open("/dev/null", O_RDONLY);
      }

      if (cpid == 0){      
        // fd = open("/dev/null", O_RDONLY);
         //if (fd == -1){
           // perror("open");
            //statusReturn = 1;
            //exit(EXIT_FAILURE);
        // }
         //else{
            fd2 = dup2(fd, 0);
            close(fd);
        // }
         if (execlp(args[0],args[0],args[1], NULL) == -1){
            perror("exec"); 
            statusReturn = 1;
            exit(EXIT_FAILURE);
         }
         //exit(EXIT_FAILURE); 
      }
      else if (cpid < 0){
         perror("fork");
         statusReturn = 1;
         return statusReturn;
      }
      else{
         printf("background pid is %i\n", cpid);
      }       
   }
   return statusReturn; 
}

int main(){
   char *input;
   char **args;
   int status = 0;
   
   //signal handler for child processes   
   struct sigaction act;
   act.sa_handler = sigHandler;
   sigaction(SIGCHLD, &act, NULL);

   do{
      printf(": ");
      fflush(stdout);
      input = readInput();

      //printf("Line 342\n");fflush(stdout);   
      //if user entered nothing continue
      //if(strcmp(input, "") == 0){
        //  continue;
      //} 
      //if an user entered #, it is a comment, ignore
      //if an user entered space ignore the line
      if(input[0] == '#' || input[0] == ' '){
         continue;
      }
      //printf("line 352\n");fflush(stdout);
      args = getArgs(input);
      status = execute(args, status);

      free(input);
      free(args);
   }while(1);

   return EXIT_SUCCESS;
}               
