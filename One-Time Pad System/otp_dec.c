/****************************************************
 * CS344 Program 4 - otp_dec                        *
 * Author: Tatsiana Clifton                         *
 * Date: 3/7/2016                                   *
 * Description: The program connects to the         *
 *              decrypting program and asks it to   *
 *              decrypt ciphertext using a passes-in*
 *              cipertext and key.                  *    
 * Usage: otp_dec <ciphertext> <key> <port>         *
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

void error(const char *msg){
   perror(msg);
   exit(1);
}

int main(int argc, char *argv[]){
   int sockfd, portno, n;
   struct hostent *server;
   char buffer[256];
   struct sockaddr_in serv_addr;
   if (argc < 3){
      fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
      exit(1);
   }
   portno = atoi(argv[2]);
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0){
      error("Cannot open socket");
   }
   server = gethostbyname(argv[1]);
   if (server == NULL){
      fprintf(stderr, "No such host\n");
      exit(0);
   }
   bzero((char *)&serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
      error("Cannot connect");
   } 
   printf("Please enter the message: ");
   bzero(buffer, 256);
   fgets(buffer, 255, stdin);
   n = write(sockfd, buffer, strlen(buffer));
   if (n < 0){
      error("Cannot write to socket");
   }
   bzero(buffer, 256);
   n = read(sockfd, buffer, 255);
   if (n < 0){
      error("Cannot read from socket");
   }
   printf("%s\n", buffer);
   close(sockfd);
   return 0;
}
