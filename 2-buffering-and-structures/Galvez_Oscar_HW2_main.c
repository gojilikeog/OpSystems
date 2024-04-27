/**************************************************************
* Class:  CSC-415-01 Summer 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub Name: gojilikeog
* Project: Assignment 2 – Buffer and Struct
*
* File: Galvez_Oscar_HW2_main.c
*
* Description:
*
* This assignment is to write a C program that populates an 
* instantiation of a structure from a header file 
* (which requires allocating memory to do so). It also requires 
* us to copy data in chucks into a buffer, and compare the result
* at the end to verify this was done correctly.
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assignment2.h"

int main(int argc, char *argv[])
{
    personalInfo *myInfo = (personalInfo *)malloc(sizeof(personalInfo));

    myInfo->firstName = argv[1];
    myInfo->lastName = argv[2];
    myInfo->studentID = 911813414;
    myInfo->level = JUNIOR;
    myInfo->languages = KNOWLEDGE_OF_C | KNOWLEDGE_OF_JAVA | KNOWLEDGE_OF_CPLUSPLUS |
                     KNOWLEDGE_OF_MIPS_ASSEMBLER;

    // strncpy used due to size limitation
    strncpy(myInfo->message, argv[3], 100);

    int personalInfoResult = writePersonalInfo(myInfo);

    char* bufferBlock = (char *) malloc(BLOCK_SIZE + 1);
    const char* nextValue = getNext();

    // Algorithm to push data into the bufferBlock.
    //
    // When condition is met, it will fill the free space in the buffer with a portion
    // of data from nextValue in order to reassign data within the buffer to the partial
    // data leftover. This must be done to continue properly copying the data being fed
    // into the file.
    //
    // totalCount: Keep track of space currently filled within buffer.
    // spaceLeft: Counts space needing to be filled in buffer.
    // textLeft: Counts data remaining to be copied into buffer from partial copy.

    size_t totalCount = 0;
    size_t spaceLeft = 0;
    size_t textLeft = 0;

    while (nextValue != NULL)
    {
        if (totalCount + strlen(nextValue) >= BLOCK_SIZE) 
        {
            spaceLeft = BLOCK_SIZE - totalCount;
            textLeft = strlen(nextValue) - spaceLeft;

            memcpy(bufferBlock + totalCount, nextValue, spaceLeft);

            commitBlock(bufferBlock);
            totalCount = strlen(nextValue) - spaceLeft;

            memcpy(bufferBlock, nextValue + spaceLeft, strlen(nextValue) - spaceLeft);
            
        } else {
            memcpy(bufferBlock + totalCount, nextValue, strlen(nextValue));
            totalCount += strlen(nextValue);
        } 

        nextValue = getNext();
    }

    // Final commit for remaining data not caught due to not filling the buffer
    commitBlock(bufferBlock);

    int result = checkIt();

    // For every corresponding mallloc, there shall be a free.
    free(bufferBlock);
    free(myInfo);

    return result;
}
