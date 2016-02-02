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
  int connections[6];
  char *type;
};

//functions prototypes
char *makeDir(); //creates a directory
void createRooms(); //generates 7 rooms with different names
void createFiles(struct Room rooms[7]); //creates 7 files with rooms

int main(){
   //declare char variable that will hold the name for directory
   char *dirName = makeDir();
   //call function to create a directory and set up it as working
   //makeDir(dirName);
   
   //change working directory to one that was created
   chdir(dirName); 
 
   //create 7 rooms
   createRooms();
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
   char *roomName[] = {"Berlin", "Amsterdam", "Vienna", "Paris", "Miami", "Venezia", "Las Vegas", "Atlanta", "London", "Sydney"};
  
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
   }
   //declare an array of 7 structs
   struct Room rooms[7];

   //add names, types, connections for every room
   int k;
   //for 7 rooms
   for(k = 0; k < 7; k++){
      //set up id
      rooms[k].id = k;
      //set up name from available names
      rooms[k].name = roomName[k];
      //temporary set type of all rooms as MID_ROOM
      rooms[k].type = "MID_ROOM";
   }

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
      }
   }
   createFiles(rooms);
}
 
//source: stackoverflow.com/questions/13432535/how-to-name-and-create-file-in-a-loop
void createFiles(struct Room rooms[7]){
   //index will be used for creating 7 files
   int i;
   FILE *fp;
   //will hold the name of file
   char fileName[15];
   for (i = 0; i < 7; i++){
      //create th ename for file according to the current index
      snprintf(fileName, sizeof(fileName), "%d.txt", i);
      //open a file
      fp = fopen(fileName, "w");
      //if there is an error while opening file
      if (fp == NULL){
         exit(1);
      }
      //add room's name to the file
      fprintf(fp, "ROOM NAME: %s\n", rooms[i].name);
      //add room's type to the file
      fprintf(fp, "ROOM TYPE: %s\n", rooms[i].type);
      //close the file 
      fclose(fp);  
   }
}
                   
