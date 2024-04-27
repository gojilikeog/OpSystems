/**************************************************************
* Class:  CSC-415-01 Summer 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub Name: gojilikeog
* Project: Assignment 1 – Command Line Arguments
*
* File: galvez_oscar_HW1_main.c
*
* Description: File prints the number of command line arguments
*              numerically while seperating them into lines.
*
**************************************************************/

#include <stdio.h>

int main ( int argc, char *argv[] )
{
  printf("There were %d arguments on the command line.\n", argc);
  
  /*For loop needed to iterate through command line arguments */
  for (int i = 0; i < argc; i++)
  {
    printf("Argument 0%d:    %s\n", i, argv[i]);
  }
  
  return 0;
}
