/****************************************************
 * CS344 Program 4 - otp_enc_d                      *
 * Author: Tatsiana Clifton                         *
 * Date: 3/7/2016                                   *
 * Description: The program runs in the background  *
 *              and performs the encoding.          *    
 * Usage: otp_enc_d <listening_port>                *
 * Sources:                                         *
 ****************************************************/                  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int numChild = 0;

void error(const char *msg){
   perror(msg);
   exit(1);
}

void handleEnc(int sock){
   int fileSize, keySize;
   int received = 0;
   int n = 0;
   //recive size of the plaintext file 
   if ((n = recv(sock, &received, sizeof(received), 0)) < -1){
      error("server cannot receive");
      exit(1);
   }
   //printf("%i\n", received);  
   fileSize = received;
   //printf("%d\n", fileSize);  
   //send the aknowlegdment message to the client
   char *msg = "I received the plaintext file size"; 
   if ((n = send(sock, msg, strlen(msg), 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   //receive plaintext file
   int sizeReceived = 0;
   char fileContent[fileSize];
   char buff[100];
   fileContent[0] = '\0';
   while (sizeReceived+1 < fileSize){
      if ((n = recv(sock, buff, sizeof(buff), 0)) == -1){
         error("Cannot receive plaintext file");
         exit(1);
      }
      else if (n == 0){
         exit(1);
      }
      else{
         strncat(fileContent, buff, n);
         sizeReceived += n;
         //printf("received size %d", sizeReceived);
      }
   }
   fileContent[sizeReceived] = '\0';
   //send the aknowlegdment message to the client that file received 
   char *str = "I received the plaintext file"; 
   if ((n = send(sock, str, strlen(str), 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   int received2 = 0;
   //recive size of the key file 
   if ((n = recv(sock, &received2, sizeof(received2), 0)) < -1){
      error("server cannot receive");
      exit(1);
   }  
   keySize = received2;
   //send the aknowlegdment message to the client
   char *msgkey = "I received the key file size"; 
   if ((n = send(sock, msgkey, strlen(msgkey), 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   //receive key file
   int sizeReceived1 = 0;
   char keyContent[keySize];
   char buff1[100];
   keyContent[0] = '\0';
   while (sizeReceived1+1 < keySize){
      if ((n = recv(sock, buff1, sizeof(buff1), 0)) == -1){
         error("Cannot receive key file");
         exit(1);
      }
      else if (n == 0){
         exit(1);
      }
      else{
         strncat(keyContent, buff1, n);
         sizeReceived1 += n;
      }
   }
   //send the aknowlegdment message to the client that key file received 
   char *str1 = "I received the key file"; 
   if ((n = send(sock, str1, strlen(str1), 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   keyContent[sizeReceived1] = '\0';
   //printf("%s\n", keyContent);

   int i;
   char encFile[fileSize]; 
   char current; 
   for (i = 0; i < fileSize; i++){
      if (fileContent[i] == ' '){
         fileContent[i] = 26;
      }
      else{
          fileContent[i] = fileContent[i] - 'A';
      }
      if (keyContent[i] == ' '){
         keyContent[i] = 26;
      }
      else{
          keyContent[i] = keyContent[i] - 'A';
      }
      current = (fileContent[i] + keyContent[i]) % 27;
      if (current == 26){
         current = ' ';
      }
      else{
          current = current + 'A';
      }
      encFile[i] = current;
   }
   encFile[i] = '\n'; 
   //send encrypted file to the client
   if ((n = send(sock, encFile, sizeof(encFile), 0)) == -1){
      error("Cannot send file");
      exit(1);
   }
}

int main(int argc, char *argv[]){
   int sockfd, newsockfd, portno;
   socklen_t clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int n;
   if (argc < 2){
      fprintf(stderr, "Port must be provided\n");
      exit(1);
   }
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      error("Cannot open socket");
   }
   bzero((char *)&serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
      error("Cannot bind");
   } 
   listen(sockfd, 5);
   while(1){
      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0){
         error("Cannot accept");
      }
      //numChild++;
      pid_t pid = fork();
      //if not forked
      if (pid < 0){
         close(newsockfd);
         perror("Fork failed"); 
      }
      //if child
      if (pid == 0){
         close(sockfd);
         if (numChild > 5){
            exit(0);
         } 
         handleEnc(newsockfd);
         close(newsockfd);
         exit(0);
      } 
      //in parent process close new child socket
      else{
         close(newsockfd);
         int status;
         //wait for all child processes to finish
         waitpid(-1, &status, WNOHANG);
      }
   }
   close (sockfd);        
   return 0;
} 
