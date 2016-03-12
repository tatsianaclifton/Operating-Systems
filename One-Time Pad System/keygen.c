/****************************************************
 * CS344 Program 4 - keygen                         *
 * Author: Tatsiana Clifton                         *
 * Date: 3/7/2016                                   *
 * Description: The program creates a key file of   *
 *              specified length.                   *    
 * Usage: keygen <keylength>                        *
 ****************************************************/           
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
       
int main(int argc, char *argv[]){
   //check for the correct number of arguments
   if (argc != 2){
      //if number of arguments is not 2, show error
      fprintf(stderr, "Usage: keygen <keylength>");
      exit (1); //exit with 1
   }
   //get the lenght of the key
   int keylength = atoi(argv[1]);
   //create array of char for the required length
   char key[keylength]; 
   //used to generate different results each time the program
   //runs; because of the time function returns distinctive values
   srand(time(NULL));
   //create array of allowed characters
   char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   int i, randIndex;
   //for key length
   for (i = 0; i < keylength; i++){
      //generate random number in range of allowed chars
      randIndex = rand() % (int)strlen(characters);
      //save random character into key
      key[i] = characters[randIndex];
   }
   //terminate the key string with NULL
   key[i] = '\0'; 
   //print key
   printf("%s\n", key);
   fflush(stdout);
   return 0;
} 
