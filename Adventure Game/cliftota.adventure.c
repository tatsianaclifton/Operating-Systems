/************************************************************
 * Name: CS344 Program 2 cliftota.adventure.c               *
 * Author: Tatsiana Clifton                                 *
 * Date: 1/26/2016                                          *
 * Description: The program creates 7 files that hold       *
 *              descriptions of the rooms and how rooms are *
 *              connected, then it offers to the player an  *
 *              interface for playing the game using those  *
 *              generated rooms. The player begins in the   *
 *              room that was assigned as "starting room"   *
 *              and wins the game upon entering the room    *
 *              that was assigned as "ending room". At the  *
 *              end the program displays the path taken by  *
 *              the player.                                 *
 ************************************************************/
#include <stdio.h> //for sprintf, NULL
#include <stdlib.h> //for exit, srand, rand
#include <sys/types.h> //for getpid
#include <sys/stat.h> //for mkdir, chdir
#include <time.h> //for time
#include <string.h>

//struct declaration; will be used for creating rooms
struct Room{
  int id;
  char *name;
  int numConnections;
  int currNumCon;
  int connections[6];
  char *conNames[6];//will be used when reading files to hold names
  char *type;
};

//functions prototypes
char *makeDir(); //creates a directory
void createRooms(); //generates 7 rooms with different names
void createFiles(struct Room rooms[7]); //creates 7 files with rooms
void play();//present game to the player

int main(){
   //declare char variable that will hold the name for directory
   char *dirName = makeDir();
   //call function to create a directory and set up it as working
   //makeDir(dirName);

   //change working directory to one that was created
   chdir(dirName);

   //create 7 rooms
   createRooms();

   //play game
   play();
}


char *makeDir(){
   //get the proccess id, will be used to create directory
   int pid = getpid();
   //generate name that will be used for creating new directory
   char *dirName = malloc(30);
   sprintf(dirName, "cliftota.rooms.%d", pid);
   //create new directory with generated name
   if (mkdir(dirName, 0777) != 0)
   {
      exit(1);
   }
   return dirName;
}

void createRooms(){
   //create array of 10 possible names for rooms,
   //will be used to pick 7 random names for rooms in the current game
   char *roomName[] = {"Berlin", "Amsterdam", "Vienna", "Paris", "Miami", "Venezia", "Boston", "Atlanta", "London", "Sydney"};

   //randomize possible names of rooms in roomName,
   //it allow to pick different names for every game
   //source: www.geeksforgeeks.org/shuffle-a-given-array

   //used to generate different results when call rand for each run of
   //the program; used function time, because it returns distinctive values
   srand(time(NULL));
   int i;
   for (i = 9; i > 0; i--){
      //choose a random index between 0 and i
      int j = rand() % (i+1);
      //swap elements at position i and j
      char *temp = roomName[i];
      roomName[i] = roomName[j];
      roomName[j] = temp;
   }//end for
   //declare an array of 7 structs
   struct Room rooms[7];

   //add names, types, connections for every room
   int k, l, m; //indexes
   //for all rooms
   for(l = 0; l < 7; l++){
      //set current connection number to zero
      rooms[l].currNumCon = 0;
      //set up id
      rooms[l].id = l;
      //set up name from available names
      rooms[l].name = roomName[l];
      //temporary set type of all rooms as MID_ROOM
      rooms[l].type = "MID_ROOM";
      //set id of connections to 8, will indicate that no
      //connection at this array position
      //since there is 7 rooms with id from 1 to 7
      for(m = 0; m < 6; m++){
         rooms[l].connections[m] = 8;
      }//end for
   }//end for

   //for 7 rooms
   for(k = 0; k < 7; k++){
      //set up connections
      //assing random number from 3 to 6 to number of connections for
      //current room
      rooms[k].numConnections = rand() % 4 + 3;
      //calculate how many additional connections needed
      int addConnections = rooms[k].numConnections - rooms[k].currNumCon;
      while(addConnections != 0 && addConnections > 0){
         //pick random room from 1 to 7
         int connected = rand() % 7;
         //check that this room is not current
         if(rooms[k].id != rooms[connected].id){
            //check that this room is not in list of connections
            if((rooms[k].connections[0] != rooms[connected].id) &&
              (rooms[k].connections[1] != rooms[connected].id) &&
              (rooms[k].connections[2] != rooms[connected].id) &&
              (rooms[k].connections[3] != rooms[connected].id) &&
              (rooms[k].connections[4] != rooms[connected].id) &&
              (rooms[k].connections[5] != rooms[connected].id)){
               int index = 0;
               //find free space for connection in the list of connections
               while(rooms[k].connections[index] != 8 && index < 7){
                  index++;
               }//end while
               //add new connections for current room
               rooms[k].connections[index] = rooms[connected].id;
               //increment number of current connections
               rooms[k].currNumCon++;
               int index2 = 0;
               //find free space for back connection
               while(rooms[connected].connections[index2] != 8 && index2 < 7){
                  index2++;
               }//end while
               //add back connection to current room
               rooms[connected].connections[index2] = rooms[k].id;
               //increase the number of connections for room
               //that was back connected to current
               rooms[connected].currNumCon++;
               //decrease the number for needed connections for current room
               addConnections--;
            }//end if
         }//end if
      }//end while
   }//end for
   //randomly pick the number of start room
   int start = rand() % 7;
   //change type of picked room to START_ROOM
   rooms[start].type = "START_ROOM";

   //used to keep track if room was picked as start
   int flag = 0;
   while (flag == 0){
      //randomly pick the number of end room
      int end = rand() % 7;
      //if randomly picked number differs from choosen for start
      if (end != start){
         //change type of the room to END_ROOM
         rooms[end].type = "END_ROOM";
         //change value to exit the while loop
         flag = 1;
      }//end if
   }//end while

   createFiles(rooms);
}//end function

//source: stackoverflow.com/questions/13432535/how-to-name-and-create-file-in-a-loop
void createFiles(struct Room rooms[7]){
   //index will be used for creating 7 files
   int i;
   FILE *fp;
   //will hold the name of file
   char fileName[15];
   for (i = 0; i < 7; i++){
      //create the name for file according to the current index
      snprintf(fileName, sizeof(fileName), "%d.txt", i);
      //open a file for writing
      fp = fopen(fileName, "w");
      //if there is an error while opening file
      if (fp == NULL){
         exit(1);
      }
      //add room's name to the file
      fprintf(fp, "ROOM NAME: %s\n", rooms[i].name);
      //add room connections;
      // using loop, because there are at least 3 connections
      int j;
      int countCon = 1;
      for (j = 0; j < 6; j++){
          //save the id of room in list of connection
          int k = rooms[i].connections[j];
          //if the room has connection
          if(rooms[i].connections[j] != 8){
             //write the connection to the file
             fprintf(fp, "CONNECTION %i: %s\n", countCon, rooms[k].name);
             countCon++;
          }
      }
      //add room's type to the file
      fprintf(fp, "ROOM TYPE: %s\n", rooms[i].type);
      //close the file
      fclose(fp);
   }
}

void play(){
   //declare an array of 7 structs
   struct Room rooms[7];
   //index will be used for reading 7 files
   int i;
   FILE *fp;
   //will hold the name of file
   char fileName[15];
   for (i = 0; i < 7; i++){
      //create th ename for file according to the current index
      snprintf(fileName, sizeof(fileName), "%d.txt", i);
      //open a file for reading
      fp = fopen(fileName, "r");
      //if there is an error while opening file
      if (fp == NULL){
         exit(1);
      }
      //count how many lines in a file in order to find
      //number of connections, 2 line are Name and Type,
      //other will be connections; source:
      //stackoverflow.com/questions/4278845/what-is-the-easiest-way-to-cout-the-newlines-in-an-ascii-file
      int ch;
      int linesCount = 0;
      while(EOF != (ch = getc(fp))){
         if (ch == '\n'){
            ++linesCount;
         }
      }
      //temporary will hold the name of the room red form file
      char name [15];
      //read the name of the room skiped 11 bytes; these byte are
      //ROOM NAME
      fseek(fp, 11, SEEK_SET);
      //save the name into temp variable
      fgets(name, 15, fp);
      //romove newline charachter; source:
      //stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
      int len = strlen(name)-1;
      if(name[len] == '\n'){
         name[len] = '\0';
      }
      //copy name into struct Room
      rooms[i].name = name;
      //set room id
      rooms[i].id = i;
      //copy number of connections into struct Room
      rooms[i].numConnections = linesCount-2;
      printf("Number %i\n", rooms[i].numConnections);
      int k;
      //for all connections
      for(k = 0; k < linesCount-2; k ++){
         //skip CONNECTION #:starting from current line
         fseek(fp, 14, SEEK_CUR);
         //save name of connection
         fgets(name, 15, fp);
         //romove newline charachter; source:
         //stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
         int len = strlen(name)-1;
         if(name[len] == '\n'){
            name[len] = '\0';
         }
         //save name of connection into struct Room
         rooms[i].conNames[k] = name;
      }
      //copy type of the room
      char type[15];
      fseek(fp, 11, SEEK_CUR);
      fgets(type, 15, fp);
      //romove newline charachter; source:
      //stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
      len = strlen(type)-1;
      if(type[len] == '\n'){
         type[len] = '\0';
      }
      //save type of room into struct Room
      rooms[i].type = type;
      printf("%i %s\n", i, rooms[i].type);
      fclose(fp);
   }
   printf("%s\n", rooms[0].type);
   printf("%s\n", rooms[1].type);
   printf("%s\n", rooms[2].type);
   printf("%s\n", rooms[3].type);
   printf("%s\n", rooms[4].type);
   printf("%s\n", rooms[5].type);
   printf("%s\n", rooms[6].type);
   int m;
   int indexOfStart;
   char *type = "START_ROOM";
   for(m = 0; m < 7; m++){
        printf("%s\n", rooms[m].type);
      if(strcmp(rooms[m].type, type) == 0){
         indexOfStart = m;
      }
   }
   printf("%i\n", indexOfStart);
}
