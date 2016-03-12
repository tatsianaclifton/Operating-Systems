/****************************************************
 * CS344 Program 4 - otp_enc                        *
 * Author: Tatsiana Clifton                         *
 * Date: start - 3/7/2016 finish - 3/12/2016        *
 * Description: The program connects to the encoding*
 *              program and asks it to perform a    *
 *              one-time pad style encryption.      *
 * Usage: otp_enc <plaintext> <key> <port>          *
 * Sources: Course material                         *
 *          www.linuxhowtos.org/c_c++/socket.html   *
 *          https://beej.us/guide/bgnet             *
 ****************************************************/     
             
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>

/*Function for printing error message and exit with 1*/
void error(const char *msg){
   perror(msg);
   exit(1);
}

/*Function that scans for invalid characters*/
int scan(char *string){
   int size = strlen(string); //holds the size of string for scan
   char allowedChar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //allowed characters
   int i;
   //loop trough every element of the string 
   for (i = 0; i < size; i++){
      //check for first occurrence of char from string in string 
      //with allowed chars; if not found return 1 
      if (strchr(allowedChar, string[i]) == 0){
         return 1;
      }
   }
   return 0; //if not invalid characters are fount, return 0
}

/*Function that sends file size to the server, gets acknowledgement
 *that size is received, sends file and gets acknowledgement that file
 *was received*/
void sendFile(FILE *fd, int size, int sock){
   char *buff; //for holding file content
   size_t result; //holding how many bytes were copied into buff from file
   int forSend, n;
   char msg[100]; //for acknowledged messages
   //allocate memory to contain file
   buff = (char*) malloc (sizeof(char)*size);
   //if allocation failed, show error and exit with 1 
   if (buff == NULL){
      fprintf(stderr, "Memory allocation error\n");
      exit(1);
   }
   //copy file into buffer
   result = fread(buff, 1, size, fd);
   //if not all file was copied into buff, show error and exit with 1
   if (result != size){
      fprintf(stderr, "Read file error\n");
      exit(1);
   }
   //add null character to the end of string with file content
   buff[size-1] = '\0';
   //check what the scan function return for buff; if 0 continue;
   //if 1 show error and exit with 1
   if (scan(buff) == 1){
      fprintf(stderr, "Invalid characters in file\n");
      exit(1);
   }
   //send file size; if failed call the error function 
   forSend = size;
   if ((n = send(sock, &forSend, sizeof(forSend), 0)) == -1){
      error("Cannot send");
   }
   //get acknowledgement that size received; if failed call error function
   memset(msg, 0, sizeof(msg));
   if ((n = recv(sock, msg, sizeof(msg), 0)) == -1){
       error("Cannot receive");
   }
   //send file; if failed call the error function
   if ((n =  send(sock, buff, strlen(buff), 0)) == -1){
      error("Cannot send file");
   }
   //get acknowledgement that file received; if failed call error function
   memset(msg, 0, sizeof(msg));
   if ((n = recv(sock, msg, sizeof(msg), 0)) == -1){
       error("Cannot receive");
   }
   fclose(fd); //close file
   free(buff); //free memory that was allocated for buff
 }

/*Function that receives the encrypted file*/
void receiveEncFile(int sock, int fileSize){
   int sizeReceived = 0; //holds how many bytes are received
   int n = 0;
   char fileContent[fileSize]; //holds encrypted file content
   char buff[100]; //holds content of one recv form the server
   fileContent[0] = '\0'; //start with null
   //while not all file is received
   while (sizeReceived+1 < fileSize){
      //continue trying to receive; if failed call the error function
      if ((n = recv(sock, buff, sizeof(buff), 0)) == -1){
         error("Cannot receive file back");
      }
      else if (n == 0){
         exit(1);
      }
      else{
         strncat(fileContent, buff, n); //add received content to 
                                        //the file content
         sizeReceived += n; //increase for n bytes the number
                            //of bytes received so far
      }
   }
   fileContent[sizeReceived-1] = '\0'; //add null to the end of file content
   printf("%s\n", fileContent); //print encrypted file content
}   


int main(int argc, char *argv[]){
   //check for correct number of arguments; if not 4 show error and exit 
   if (argc != 4){
      fprintf(stderr, "Usage: %s <plaintext> <key> <port>\n", argv[0]);
      exit(1);
   }
   int sockfd, portno, n; //for socket, port number and number of bytes recv
   struct hostent *server; 
   char buffer[256];
   struct sockaddr_in serv_addr;
   portno = atoi(argv[3]); //convert string from argv 
                           //to integer to get port number
   sockfd = socket(AF_INET, SOCK_STREAM, 0); //open stream socket
   //if failed call the error function
   if (sockfd < 0){
      error("Cannot open socket");
   }
   //define server for connection as localhost
   server = gethostbyname("localhost");
   if (server == NULL){
      fprintf(stderr, "No such host\n");
      exit(0);
   }
   //fill the socket address structure
   bzero((char *)&serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   //try connect to socket; if failed show error and exit with 2
   if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
      fprintf(stderr, "Cannot connect to port %d\n", portno);
      exit(2);
   }
   //get acknowledgement form the server that client connected to the correct server
   int num;
   if ((n = recv(sockfd, &num, sizeof(num), 0)) < -1){
      error("Client cannot get acknowledgement");
   }
   //check that acknowledgement from server is 5; it will mean 
   //that client connected to correct server; otherwise show error,
   //close socket and exit with 2
   else{
      num = ntohs(num);
      if (num != 5){
         fprintf(stderr, "Client cannot use this server\n");
         close(sockfd);
         exit(2);  
       } 
   }
   //get file name
   char *plaintext = argv[1];
   //get rid of new line character at the end of file name
   int ln = strlen(plaintext) - 1;
   if (plaintext[ln] == '\n'){
      plaintext[ln] = '\0';
   }
   int textSize; //holds file size
   FILE *fd1; //create file descriptor pointer
   //open file for reading
   fd1 = fopen(plaintext, "r");
   //if cannot open the file, close socket and show error message
   if (!fd1){
      fprintf(stderr, "Cannot open the file\n");
      close(sockfd);
      exit(1); 
   }
   //get key file name
   char *key = argv[2];
   //get rid of new line character at the end of file name
   int len = strlen(key) - 1;
   if (key[len] == '\n'){
      key[len] = '\0';
   }
   int keySize; //holds key file size
   FILE *fd2; //create file descriptor pointer for key file
   //open key file for reading
   fd2 = fopen(key, "r");
   //if cannot open the file, close socket and show error message
   if (!fd2){
      fprintf(stderr, "Cannot open the file\n");
      close(sockfd);
      exit(1); 
   }
   //if files were opened, send them if key > plaintext 
   else{
      //get file sizes
      fseek(fd2, 0, SEEK_END); //find the end of key file
      keySize = ftell(fd2); //get key file size
      rewind(fd2); //return to the beginning of the key file
      fseek(fd1, 0, SEEK_END); //find the end of file
      textSize = ftell(fd1); //get file size
      rewind(fd1); //return to the beginning of the file
      //show error if key file size is less than file size
      if (keySize < textSize){
         fprintf(stderr, "The key file cannot be shorter than the plaintext.\n");
         fclose(fd1); //close file
         fclose(fd2); //close key file
         exit(1); //exit with 1
      }
      //if key file is big enough, send files to the server
      else{
         sendFile(fd1, textSize, sockfd);
         sendFile(fd2, keySize, sockfd);
      }
   }
   //receive encrypted file from the server
   receiveEncFile(sockfd, textSize);
   //close socket
   close(sockfd);
   return 0;
}
