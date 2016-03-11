/****************************************************
 * CS344 Program 4 - otp_enc                        *
 * Author: Tatsiana Clifton                         *
 * Date: 3/7/2016                                   *
 * Description: The program connects to the encoding*
 *              program and asks it to perform a    *
 *              one-time pad style encryption.      *
 * Usage: otp_enc <plaintext> <key> <port>          *
 * Sources: Course material                         *
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

void error(const char *msg){
   perror(msg);
   exit(1);
}

int scan(char *string){
   int size = strlen(string);
   char allowedChar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   int i;
   for (i = 0; i < size; i++){
      //check for first occurence of char from string in string 
      //with allowed chars; if not found return 1 
      if (strchr(allowedChar, string[i]) == 0){
         return 1;
      }
   }
   return 0;
}

void sendFile(FILE *fd, int size, int sock){
   char *buff;
   size_t result;
   int forSend, n;
   char msg[100];//for aknowledgemnt messages
   //allocate memory to contain file
   buff = (char*) malloc (sizeof(char)*size);
   if (buff == NULL){
      fprintf(stderr, "Memory allocation error\n");
      exit(1);
   }
   //copy file into buffer
   result = fread(buff, 1, size, fd);
   if (result != size){
      fprintf(stderr, "Readinf error\n");
      exit(1);
   }
   buff[size-1] = '\0';
   //printf("%s", buff); 
   if (scan(buff) == 1){
      fprintf(stderr, "Files contain invalid characters.\n");
      exit(1);
   }
   //send file size 
   forSend = size;
   if ((n = send(sock, &forSend, sizeof(forSend), 0)) == -1){
      error("Cannot send");
      exit(1); 
   }
   //get aknowlegdment
   memset(msg, 0, sizeof(msg));
   if ((n = recv(sock, msg, sizeof(msg), 0)) == -1){
       error("Cannot receive");
       exit(1);
   }
   //printf("%s\n", msg);
   //send file
   if ((n =  send(sock, buff, strlen(buff), 0)) == -1){
      error("Cannot send file");
      exit(1);
   }
   //get aknowlegdment that file received
   memset(msg, 0, sizeof(msg));
   if ((n = recv(sock, msg, sizeof(msg), 0)) == -1){
       error("Cannot receive");
       exit(1);
   }
   fclose(fd);
   free(buff); 
   //printf("Transfer completed\n");
}

void receiveEncFile(int sock, int fileSize){
   int sizeReceived = 0;
   int n = 0;
   char fileContent[fileSize];
   char buff[100];
   fileContent[0] = '\0';
   while (sizeReceived+1 < fileSize){
      if ((n = recv(sock, buff, sizeof(buff), 0)) == -1){
         error("Cannot receive file back");
         exit(1);
      }
      else if (n == 0){
         exit(1);
      }
      else{
         strncat(fileContent, buff, n);
         sizeReceived += n;
      }
   }
   //printf("size is %d\n", sizeReceived); 
   fileContent[sizeReceived-1] = '\0';
   printf("%s\n", fileContent);
}   

int main(int argc, char *argv[]){
   //check for correct number of arguments
   if (argc != 4){
      fprintf(stderr, "Usage: %s <plaintext> <key> <port>\n", argv[0]);
      exit(1);
   }
   int sockfd, portno, n;
   struct hostent *server;
   char buffer[256];
   struct sockaddr_in serv_addr;
   portno = atoi(argv[3]);
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0){
      error("Cannot open socket");
   }
   server = gethostbyname("localhost");
   if (server == NULL){
      fprintf(stderr, "No such host\n");
      exit(0);
   }
   bzero((char *)&serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
      fprintf(stderr, "Cannot connect to port %d\n", portno);
      exit(2);
   } 
   char *plaintext = argv[1];
   //get rid of new line character at the end of file name
   int ln = strlen(plaintext) - 1;
   if (plaintext[ln] == '\n'){
      plaintext[ln] = '\0';
   }
   int textSize;
   FILE *fd1;
   fd1 = fopen(plaintext, "r");
   //if cannot open the file, show error message
   if (!fd1){
      fprintf(stderr, "Cannot open the file\n");
      close(sockfd);
      exit(1); 
   }
   char *key = argv[2];
   //get rid of new line character at the end of file name
   int len = strlen(key) - 1;
   if (key[len] == '\n'){
      key[len] = '\0';
   }
   int keySize;
   FILE *fd2;
   fd2 = fopen(key, "r");
   //if cannot open the file, show error message
   if (!fd2){
      fprintf(stderr, "Cannot open the file\n");
      close(sockfd);
      exit(1); 
   }
   //if files were opened, send them if key > plaintext 
   else{
      //get file size
      fseek(fd2, 0, SEEK_END);
      keySize = ftell(fd2);
      rewind(fd2);
      fseek(fd1, 0, SEEK_END);
      textSize = ftell(fd1);
      rewind(fd1);
      if (keySize < textSize){
         fprintf(stderr, "The key file cannot be shorter than the paintext.\n");
         fclose(fd1);
         fclose(fd2);
         exit(1);
      }
      else{
         sendFile(fd1, textSize, sockfd);
         sendFile(fd2, keySize, sockfd);
      }
   }
   receiveEncFile(sockfd, textSize);
   close(sockfd);
   return 0;
}
