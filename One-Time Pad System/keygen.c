/****************************************************
 * CS344 Program 4 - keygen                         *
 * Author: Tatsiana Clifton                         *
 * Date: 3/7/2016                                   *
 * Description: The program creates a key file of   *
 *              specified length.                   *    
 * Usage: keygen <keylength>                        *
 * Sources:                                         *
 ****************************************************/                  
int main(int argc, char *argv[]){
   if (argc != 2){
      fprintf(stderr, "Usage: keygen <keylength>");
      exit (1); 
   }
   return 0;
} 
