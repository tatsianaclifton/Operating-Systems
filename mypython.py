#!/usr/bin/python

#CS344 Program 5                                    #
#Author: Tatsiana Clifton                           #    
#Date: 3/1/2016                                     #     
#Description: The program creates three files,      #
#             writes 10 random characters to each,  #
#             prints the content of files. It prints#
#             two random integers and their sum.    #

import random
import string

#create array of file names
fileNames = ["fileOne", "fileTwo", "fileThree"]

#for each file name
for name in fileNames:
   #create a file with this name 
   fo = open(name, "w")   
   #generate string of 10 random lowercase characters
   s = ''.join([random.choice(string.ascii_lowercase) for y in range(10)])
   #print generated string
   print 'Content of ' + name + ' : ' + s
   #write string into the file
   fo.write(s)
   #close the file
   fo.close()

#generate two random numbers
numberOne = random.randint(1,42)
numberTwo = random.randint(1,42)

#print two numbers that were randomly generated
print 'First number: ' + str(numberOne) + '\nSecond number: ' + str(numberTwo)
#print the product of these numbers
print 'The product of two numbers: ' + str(numberOne*numberTwo)
