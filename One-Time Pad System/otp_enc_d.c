/****************************************************
 * CS344 Program 4 - otp_enc_d                      *
 * Author: Tatsiana Clifton                         *
 * Date: 3/7/2016                                   *
 * Description: The program runs in the background  *
 *              and performs the encoding.          *    
 * Usage: otp_enc_d <listening_port>                *
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
#include <netinet/in.h>

/*Function for printing error message and exit with 1*/
void error(const char *msg){
   perror(msg);
   exit(1);
}

/*Function that receives the plain text and the key files,
 *performs encryption and sends encrypted file back to the client*/
void handleEnc(int sock){
   int fileSize, keySize; //hold file size and key file size
   int received = 0; //holds how many bytes received
   int n = 0; //holds number of bytes for current receive operation
   //receive size of the plaintext file; if failed show error 
   if ((n = recv(sock, &received, sizeof(received), 0)) < -1){
      error("server cannot receive");
      exit(1);
   }  
   fileSize = received;
   //send the acknowledgement message to the client; if failed call error 
   char *msg = "I received the plaintext file size"; 
   if ((n = send(sock, msg, strlen(msg), 0)) == -1){
      error("server cannot send");
   }  
   //receive plaintext file
   int sizeReceived = 0;
   char fileContent[fileSize]; //holds all file content
   char buff[100]; //holds current received characters
   fileContent[0] = '\0'; //start with null
   //while not all file received
   while (sizeReceived+1 < fileSize){
      //continue trying to receive; if failed call the error function 
      if ((n = recv(sock, buff, sizeof(buff), 0)) == -1){
         error("Cannot receive plaintext file");
      }
      else if (n == 0){
         exit(1);
      }
      else{
         strncat(fileContent, buff, n);//add curr content to final content
         sizeReceived += n; //increase for n number of bytes received
      }
   }
   //add null to the end of file content
   fileContent[sizeReceived] = '\0';
   //send the acknowledgement message to the client that file received 
   char *str = "I received the plaintext file"; 
   if ((n = send(sock, str, strlen(str), 0)) == -1){
      error("server cannot send");
   }  
   int received2 = 0;
   //receive size of the key file 
   if ((n = recv(sock, &received2, sizeof(received2), 0)) < -1){
      error("server cannot receive");
   }  
   keySize = received2;
   //send the acknowledgement message to the client
   char *msgkey = "I received the key file size"; 
   if ((n = send(sock, msgkey, strlen(msgkey), 0)) == -1){
      error("server cannot send");
   }  
   //receive key file
   int sizeReceived1 = 0;
   char keyContent[keySize]; //holds key file content
   char buff1[100]; //holds current content
   keyContent[0] = '\0'; //start with null
   //while not all key file received
   while (sizeReceived1+1 < keySize){
      //continue trying to receive
      if ((n = recv(sock, buff1, sizeof(buff1), 0)) == -1){
         error("Cannot receive key file");
      }
      else if (n == 0){
         exit(1);
      }
      else{
         strncat(keyContent, buff1, n); //add curr content to final content
         sizeReceived1 += n; //increase number of bytes received for n
      }
   }
   //send the acknowledgement message to the client that key file received 
   char *str1 = "I received the key file"; 
   if ((n = send(sock, str1, strlen(str1), 0)) == -1){
      error("server cannot send");
   }  
   keyContent[sizeReceived1] = '\0'; //add null to the end of key file

   //perform encryption
   int i;
   char encFile[fileSize]; //holds encrypted file 
   char current; 
   //for every character in received file
   for (i = 0; fileContent[i] != '\n'; i++){
      //if character is space, assign 26 to it
      if (fileContent[i] == ' '){
         fileContent[i] = 26;
      }
      //otherwise it equals char minus 65 or 'A', since 'A' = 65
      else{
          fileContent[i] = fileContent[i] - 'A';
      }
      //if character in key file is space, assign 26 to it
      if (keyContent[i] == ' '){
         keyContent[i] = 26;
      }
      //otherwise it equals char minus 65 or 'A', since 'A' = 65
      else{
          keyContent[i] = keyContent[i] - 'A';
      }
      //calculate encrypted character using mod 27
      current = (fileContent[i] + keyContent[i]) % 27;
      if (current == 26){
         current = ' ';
      }
      else{
          current = current + 'A';
      }
      encFile[i] = current; //add calculated character to encrypted file
   }
   encFile[i] = '\0'; //add null char to the end
   //send encrypted file to the client
   if ((n = send(sock, encFile, sizeof(encFile), 0)) == -1){
      error("Cannot send file");
   }
}


int main(int argc, char *argv[]){
   int sockfd, newsockfd, portno; //for parent and child sockets and port 
   socklen_t clilen;
   struct sockaddr_in serv_addr, cli_addr;
   int n;
   //check for correct number of arguments
   if (argc < 2){
      fprintf(stderr, "Port must be provided\n");
      exit(1);
   }
   //open socket; if failed call the error function
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      error("Cannot open socket");
   }
   //fill address struct
   bzero((char *)&serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   //bind opened socket to port 
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
      error("Cannot bind");
   } 
   listen(sockfd, 5); //listen on opened socket
   while(1){
      clilen = sizeof(cli_addr); //client address size
      //accept connection from the client
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0){
         error("Cannot accept");
      }
	  //create another process
      pid_t pid = fork();
      //if not forked, close socket and show error
      if (pid < 0){
         close(newsockfd);
         perror("Fork failed"); 
      }
      //if child
      if (pid == 0){
         close(sockfd); //close parent socket
         int authNum = 5; //authentication number for this server
         authNum = htons(authNum); //convert number to network number
         //send authentication number 
         if ((n = send(newsockfd, &authNum, sizeof(authNum), 0)) == -1){
            fprintf(stderr, "Server cannot sent authentication\n");
            close(newsockfd); //close socket
            exit(1); 
         }
         handleEnc(newsockfd); //do encryption
         close(newsockfd); //close socket
         exit(0);
      } 
      //in parent process close new child socket
      else{
         close(newsockfd); //close child socket
         int status;
         //wait for all child processes to finish
         waitpid(-1, &status, WNOHANG);
      }
   }
   close (sockfd); //close socket       
   return 0;
} 
