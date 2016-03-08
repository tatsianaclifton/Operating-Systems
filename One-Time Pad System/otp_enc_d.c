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

void error(const char *msg){
   perror(msg);
   exit(1);
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
   clilen = sizeof(cli_addr);
   newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
   if (newsockfd < 0){
      error("Cannot accept");
   }
   bzero(buffer, 256);
   n = read(newsockfd, buffer, 255);
   if (n < 0){
      error("Cannot read from socket");
   }
   printf("Here is the message: %s\n", buffer);
   n = write(newsockfd, "I got your message", 18);
   if (n < 0){
      error("Cannot write to socket");
   }
   close(newsockfd);
   close(sockfd);
   return 0;
} 
