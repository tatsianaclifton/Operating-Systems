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
   int received, n;
   //recive size of the plzintext file 
   if ((n = recv(sock, &received, sizeof(received), 0)) < -1){
      error("server cannot receive");
      exit(1);
   }
   printf("%i\n", received);  
   fileSize = ntohs(received);
   printf("%d\n", fileSize);  
   //send the aknowlegdment message to the client
   char *string = "I received the plaintext file size"; 
   int len = strlen(string); 
   if ((n = send(sock, string, len, 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   //receive plaintext file
   int sizeReceived;
   char fileContent[fileSize];
   char buff[100];
   fileContent[0] = '\0';
   //while (sizeReceived < fileSize){
      if ((n = recv(sock, buff, sizeof(buff), 0)) <= -1){
         error("Cannot receive plaintext file");
         exit(1);
      }
      //else{
         //strncat(fileContent, buff, n);
         //sizeReceived += n;
      //}
   //}
   fileContent[sizeReceived] = '\0';
   //send the aknowlegdment message to the client that file received 
   char *str = "I received the plaintext file"; 
   int l = strlen(str); 
   if ((n = send(sock, str, l, 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   int received2;
   //recive size of the key file 
   if ((n = recv(sock, &received2, sizeof(received2), 0)) < -1){
      error("server cannot receive");
      exit(1);
   }  
   printf("2%s\n", fileContent);
   keySize = ntohs(received2);
   printf("3%s\n", fileContent);
   //send the aknowlegdment message to the client
   char *string1 = "I received the key file size"; 
   printf("4%s\n", fileContent);
   int ln = strlen(string1); 
   if ((n = send(sock, string, ln, 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   //receive key file
   int sizeReceived1;
   char keyContent[keySize];
   char buff1[100];
   keyContent[0] = '\0';
   while (sizeReceived1 < keySize){
      if ((n = recv(sock, buff1, sizeof(buff1), 0)) <= -1){
         error("Cannot receive key file");
         exit(1);
      }
      else{
         strncat(keyContent, buff1, n);
         sizeReceived1 += n;
      }
   }
   //send the aknowlegdment message to the client that key file received 
   char *str1 = "I received the key file"; 
   int l1 = strlen(str1); 
   if ((n = send(sock, str1, l1, 0)) == -1){
      error("server cannot send");
      exit(1);
   }  
   keyContent[sizeReceived1] = '\0';
   printf("%s\n", keyContent);
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
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0){
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
      numChild++;
      pid_t pid = fork();
      //if not forked
      if (pid < 0){
         perror("Fork failed"); 
      }
      //if child
      if (pid == 0){
         close(sockfd);
         if (numChild > 5){
            exit(0);
         } 
         handleEnc(newsockfd);
      } 
      //in parent process close new child socket
      else{
         close(newsockfd);
         int status;
         //wait for all child processes to finish
         waitpid(-1, &status, WNOHANG);
      }
   }
   close (newsockfd);        
   return 0;
} 
