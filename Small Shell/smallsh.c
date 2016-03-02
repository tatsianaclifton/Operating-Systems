/*************************************************************
 * CS344 Program 3 - smallsh                                 *
 * Author: Tatsiana Clifton                                  *
 * Date: 2/17/2016  Finished: 2/28/2016                      *
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
 *          Lectures Week 8                                  *
 *          brennan.io/2015/01/16/write-a-shell-in-c         *  
 *************************************************************/ 

#include <stdio.h> //for fprintf, stderr, perror
#include <stdlib.h> //for malloc, free, exit, getenv
#include <unistd.h> //for chdir, fork, exec, pid_t
#include <sys/wait.h> //for witpid
#include <string.h> //for strcmp, strtok
#include <signal.h> //for sigaction 
#include <sys/types.h> //for pid_t
#include <fcntl.h> //for open
#include <termios.h> //for tcflush

#define MAXSIZE 2048 //used limit the length of a command 
#define MAXARG 512 //used to limit number of arguments  
#define DELIM " \n\t\r\a" //delimiter for commands

int signo; //will hold the signal number
int bg = 0; //flag for background processes; if 1 it is bg process


//parse the input string into array of commands
char **getArgs(char *input){
   int bufsize = MAXARG; //save the max number of arguments into variable
   char **argsArr = malloc(bufsize * sizeof(char*)); //allocate memory
   char *token; //will hold tokens from input separated by space
   int i = 0; 
   //check if memory allocated
   if (!argsArr){
      fprintf(stderr, "allocation error\n"); //if not, show error
      fflush(stdout);
      fflush(stdin);
      exit(1); //exit
   }
   token = strtok (input, DELIM); //get token from input
   //while there are tokens
   while (token != NULL){
      //if token is &, break and set flag for bg process
      if (strcmp(token, "&") == 0){
         bg = 1;
         break;
      }
      //add token to array of commands
      argsArr[i] = token;
      token = strtok (NULL, DELIM); 
      i++; 
   }
   //add NULL as the last element of array of commands
   argsArr[i] = NULL;
   //return array 
   return argsArr;   
}


//execute build in command: cd, status
int execute(char **args, int status){
   //if nothing is entered, return
   if (args[0] == NULL){
      return 1; 
   }
   //cd command
   if (strcmp("cd", args[0]) == 0){
      //if directory is specified change to this directory
      if (args[1] != NULL){
         chdir(args[1]);
         return 1;
      }
      //if directory is not specified change to home directory
      else{
         chdir(getenv("HOME"));
         return 1;
      }
   }
   //status command
   if (strcmp("status", args[0]) == 0){
      //if status is 2, process was terminated by signal
      if (status == 2){
         //print terminated message with signal number
         printf("terminated by signal %d\n", signo);
         fflush(stdout); //flush out the output buffer
         fflush(stdin);
      }
      //otherwise print exit value
      else{
         printf("exit value %d\n", status);
         fflush(stdout); //flush out the output buffer
         fflush(stdin);
      }   
   }
   //not built-in commands
   else { 
      return executeCommand(args);
   }      
}


//execute any non-built in commands
int executeCommand(char **args){
   int status = 0;
   int statusReturn = 0;
   pid_t cpid, wpid;
   int finput = 0; //flag for input,  if 1 input should be redirected
   int foutput = 0; //flag for output, if 1 output should be redirected

   //while there are arguments check for existence of < and >
   if (args[1] != NULL){  
      if (strcmp(args[1], "<") == 0){
         finput = 1; //set flag if input redirection          
      }
      else if (strcmp(args[1], ">") == 0){
         foutput = 1; //set flag for output redirection
      } 
   }

   int fd, fd2;
   //if it is a foreground process
   if (bg == 0){
       //if input and output are not redirected  
       if (finput == 0 && foutput == 0){ 
          //create a new process by duplicating the current process
          cpid = fork(); 
          //if it is a child process
          if (cpid == 0){
             //set signal handler to default that it can catch CTRL+C
             struct sigaction act;
             act.sa_handler = SIG_DFL;
             sigaction(SIGINT, &act, NULL);
             //execute command or show error   
             if (execvp(args[0], args) == -1){ 
                perror("exec");
                statusReturn = 1;
                exit(1);
             }   
          }
          //if fork failed show error 
          else if (cpid < 0){
             perror("fork()");
             return 1;
          }
          //if it is a parent process
          else{
             //handler for parent process to ignore CTRL-C
             struct sigaction act;
             act.sa_handler = SIG_DFL;
             act.sa_handler = SIG_IGN;
             sigaction(SIGINT, &act, NULL);   
             //wait for child process             
             waitpid(cpid, &status, 0);
             //save not zero status if child was terminated with exit
             statusReturn = WEXITSTATUS(status);
             //if terminated by signal show termination message
             if (WIFSIGNALED(status)){
                signo = WTERMSIG(status);
                printf("terminated by signal %d\n", signo);
                fflush(stdout);
                fflush(stdin); 
                statusReturn = 2;
             }
          } 
       }
       //redirect standard input
       else if (finput != 0 && foutput == 0){
          //create a new process by duplicating the current process
          cpid = fork();
          //if it is a child process
          if (cpid == 0){
             //set signal handler to default that it can catch CTRL+C
              struct sigaction act;
              act.sa_handler = SIG_DFL;
              sigaction(SIGINT, &act, NULL);
              //open input file              
              fd = open(args[finput+1], O_RDONLY);
              //if it was not opened show error and exit
              if (fd == -1){
                 perror("open");   
                 statusReturn = 1;
                 exit(1);
              }
              //redirect standard input into file  
              fd2 = dup2(fd,0);
              //execute command ot show error and exit
              if (execlp(args[0], args[0], NULL) == -1){ 
                  perror("exec");
                  statusReturn = 1;
                  exit(1);  
              }
          }
          //if fork failed show error
          else if (cpid < 0){
             perror("fork()");
             statusReturn = 1;
             return statusReturn;
          }
          //if it is a parent process 
          else{
             //wait for child
             wpid = waitpid(cpid, &status, 0);
             //get status if it was exited with exit 
             statusReturn = WEXITSTATUS(status);
          } 
          close(fd); //close the file
      }
      //if output is redirected
      else {
         //create a new process
         cpid = fork();
         //if it is a child
         if (cpid == 0){ 
            //set signal handler to default
            struct sigaction act;
            act.sa_handler = SIG_DFL;
            sigaction(SIGINT, &act, NULL);
            //open output file
            fd = open(args[foutput+1], O_WRONLY|O_CREAT|O_TRUNC,0644);
            //show error if failed to open
            if (fd == -1){
               perror("open");
               statusReturn = 1;
               exit(1);
             }
             //redirect standard output to file
             fd2 = dup2(fd,1);
             //execute command or show error and exit
             if (execlp(args[0], args[0], NULL) == -1){ 
                perror("exec");
                statusReturn = 1;
                exit(1);
             }
          }
          //if fork failed show error 
          else if (cpid < 0){
             perror("fork()");
             statusReturn = 1;
             return statusReturn;
          }
          //parent process
          else{
             //wait for child
             wpid = waitpid(cpid, &status, 0);
             //get status if terminated with exit
             statusReturn = WEXITSTATUS(status);
          } 
          close(fd); //close file  
       }
   }
   //background process
   else if (bg != 0){
      //start a new process
      cpid = fork();
      //if input is redirected   
      if (finput == 1){
         //open file
         fd = open(args[finput+1], O_RDONLY, 0644);
         //if failed to open, show error and exit
         if (fd == -1){
            perror("open");
            statusReturn = 1;
            exit(1);
          }
      }
      //if output is redirected
      if (foutput == 1){
         //open file
         fd = open(args[foutput+1], O_WRONLY|O_TRUNC|O_CREAT, 0644);
         //if failed to open, show error and exit
         if (fd == -1){
            perror("open");
            statusReturn = 1;
            exit(1);
          }
          //redirect output
          dup2(fd, 1);
       }
      //if no file redirection, open the null device    
      else{
         fd = open("/dev/null", O_RDONLY);
         //if failed to open, show error and exit 
         if (fd == -1){
            perror("open");
            statusReturn = 1;
            exit(1);
          }
      }
      //it is a child
      if (cpid == 0){
            //redirect input     
            fd2 = dup2(fd, 0);
            close(fd); //close file
            bg = 0;
         //execute command or show error and exit
         if (execvp(args[0],args) == -1){
            perror("exec"); 
            statusReturn = 1;
            exit(1);
         }
      }
      //if fork failed show error
      else if (cpid < 0){
         perror("fork");
         statusReturn = 1;
         return statusReturn;
      }
      //parent process
      else{
         printf("background pid is %d\n", cpid); //print bg process id
         fflush(stdout); //flush out output buffer
         fflush(stdin);
         bg = 0;
      }       
   }
   return statusReturn; //return status
}


//signal handler for child processes
void sigHandler(){
   int status;
   int signo1;
   pid_t cpid;
   char msg[100]; 
   char sigNum[5];
   char statusNum[3];
   char pid[15];  //for child pid string
   //for all child processes wait for any child process to change state
   while ((cpid = waitpid(-1, &status, WNOHANG)) > 0){
      snprintf(pid, 15, "%d", cpid); //create string from child pid
      //construct the output message for done bg process
      strncpy(msg, "background pid ", 100);
      strcat(msg, pid);
      strcat(msg, " is done:  ");
      //if a child process is terminated by signal
      if (WIFSIGNALED(status)){
          signo1 = WTERMSIG(status); //save signal number
          snprintf(sigNum, 5, "%d", signo1); //convert to string this number
          //construct termination message   
          strcat(msg, "terminated by signal ");
          strcat(msg, sigNum);
          strcat(msg, "\n");
          //write message to standard output  
          write(1, msg, 100);
      }
      //if a child process is finished normal
      else{
         status = WEXITSTATUS(status); //save status of the process
         snprintf(statusNum, 3, "%d", status); //convert to string
         //construct exit message
         strcat(msg, "exit value ");
         strcat(msg, statusNum);
         strcat(msg, "\n");
         //write message to standard output
         write(1, msg, 100);    
      }
      continue;
   } 
}


int main(){
   char quit = 0; //will be use to control loop
   char input[MAXSIZE] = ""; //will hold user's command input
   char **args; //will hold array of pointers to commands
   int status = 0; //will hold status of last foreground command
   
   //signal handler for child processes   
   struct sigaction act;
   act.sa_handler = sigHandler;
   sigaction(SIGCHLD, &act, NULL);

   //run the loop 
   while (quit == 0){
      tcflush(0, TCIOFLUSH); //clear input, output
      strncpy(input, "", MAXSIZE - 1); //clear input variable
      printf(": "); //print the prompt for command line
      fgets(input, 2048, stdin); //get user's command
      fflush(stdout); //flush out output buffer
           
      //remove new line character
      size_t len = strlen(input) - 1;
      if (input[len] == '\n'){
         input[len] = '\0';
      }

      //if an user entered #, it is a comment, ignore
      //if an user entered space ignore the line
      if(input[0] == '#' || input[0] == ' '){
         continue;
      }
      //if an user entered exit, exit  
      if (strcmp(input, "exit") == 0){
         quit = 1;
         exit(0);
      }
      //parse input into commands
      args = getArgs(input);
      //get the status of last foreground command
      status = execute(args, status);
      // free array of commands
      free(args);
   }
   //finish the program   
   return EXIT_SUCCESS;
}               
