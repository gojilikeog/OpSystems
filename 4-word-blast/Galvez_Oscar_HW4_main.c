/**************************************************************
* Class:  CSC-415-01 Spring 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub ID: gojilikeog
* Project: Assignment 4 – Word Blast
*
* File: Galvez_Oscar_HW4_main.c
*
* Description: This program is to accept a text file, in this case WarandPeace.txt,
*              read the file, and count the number of occurrences of words with 6 or
*              letters. This is to be accomplished with multithreading. Once that is
*              complete, it will process the list and display the top 10 most occuring
*              words in the text file, in descending order.
*
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


// MIN_WORD_SIZE refers to minimum character length for word
// MAX_SIZE refers to maximum character length planned for program
//
// TABLE_SIZE refers to size of array to hold words
// 14000 set due to selected text requiring less than 14000 words
// for this program to work.

#define MIN_WORD_SIZE 6
#define MAX_SIZE 15
#define TABLE_SIZE 14000

// Text delimitiers
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

// The following variables are being defined with a global scope
// to be used in the runner function for multithreading
//
// currentWords -   Tracks number of words currently stored for proper
//                  use of conditionals dependant on number of valid entries
//
// remainingBytes - Use in calculating left over bytes from partition of
//                  bytes due to threads.
//
// fileDescriptor - Variable to hold return value of open()
//
// blockSize -      Holds block size partition dependent on number of threads
//                  specified by command line argument.
int blockSize, fileDescriptor;
int remainingBytes = 0;
int currentWords = 0;
pthread_mutex_t criticalLock;

typedef struct wordInfo {
    char* word;
    int count;
} wordInfo;

// Declaration of array with wordInfo struct
wordInfo wordList[TABLE_SIZE];

// *runner function will be code block being run by threads
void *runner(void *param) {
    char * tokenPointer;
    char * saveptr1;

    // Text buffer - calculated blockSize per thread accounting for leftover
    //               bytes from variables threads
    char* bufferBlock = malloc(blockSize + remainingBytes + 1);

    // Passes in specified blockSize + left over bytes from partitioning into buffer
    read(fileDescriptor, bufferBlock, blockSize + remainingBytes);

    // Parse the string into sub strings based on delimitiers defined
    tokenPointer = strtok_r(bufferBlock, delim, &saveptr1);

    // iterator declared outside for loop for use in duplicate entry if statement
    int iterator = 0;

    while (tokenPointer != NULL) {
        // Per specifications - if word length is 6 or more characters we process
        if (strlen(tokenPointer) >= MIN_WORD_SIZE) {

            // Checks all entries to duplicate word - iterates count of word if detected
            for (iterator = 0; iterator < currentWords; iterator++) {
                // Non-case sensitive comparison
                if (strcasecmp(tokenPointer, wordList[iterator].word) == 0) {

                    // CRITICAL SECTION - Protecting with mutex lock
                    pthread_mutex_lock(&criticalLock);
                    wordList[iterator].count++;
                    pthread_mutex_unlock(&criticalLock);
                    // END CRITICAL SECTION

                    break; // Move on to next word in tokenizer
                }
            }

            // Statement to add word to list after checking all current entries in
            // previous loop.
            //
            // If statement in place to prevent duplicate entry - break doesn't skip this
            // Non-case sensitive comparison
            if (strcasecmp(tokenPointer, wordList[iterator].word) != 0) {

                // CRITICAL SECTION - Protecting with mutex lock
                pthread_mutex_lock(&criticalLock);

                // strcpy must be used to avoid memory leak - direct assignment
                // prevents use for free()
                strcpy(wordList[currentWords].word, tokenPointer);

                wordList[currentWords].count++;
                currentWords++; // Increment for tracking words in list
                printf("Word added: %s\n", tokenPointer);
                pthread_mutex_unlock(&criticalLock);
                // END CRITICAL SECTION
            }
        }

        // Incremenent tokenizer
        tokenPointer = strtok_r(NULL, delim, &saveptr1);
    }

    free(bufferBlock); // For every malloc, there shall be a corresponding free
    bufferBlock = NULL; // Defensive - Protect from dangling pointer
}

int main (int argc, char *argv[])
    {
    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures
    
    // open function returns nonnegative int if successful - Read-only mode
    fileDescriptor = open(argv[1], O_RDONLY);

    // If open fails, report error and exit program.
    if (fileDescriptor == -1) {
        printf("Open failed. Error #: %d\n", fileDescriptor);
        perror("Program");
        exit(fileDescriptor);
    }

    // Conversion of command line argument to int for blockSize calculation
    int threadCount = atoi(argv[2]);
    // Returns file size in bytes for calculation of blockSize for multithreading
    int fileSize = lseek(fileDescriptor, 0, SEEK_END);
    // Reset pointer to start of file to proper data processing
    lseek(fileDescriptor, 0, SEEK_SET);
    // Calculate blockSize for multithreading data processing 
    blockSize = fileSize / threadCount;

    // Calculate left over bytes from partitioning due to number of threads specificed 
    // from command line argument. Used in malloc and read functions.
    remainingBytes = fileSize % threadCount;

    int err; // General error detection int - Used for error reporting

    err = pthread_mutex_init(&criticalLock, NULL);
    //Reporting failure of mutex if occured - Abort program
    if (err != 0) {
        printf("mutex_init has failed. Error #%d\n", err);
        exit(err);
    }

    // Initialize array in structure
    for (int i = 0; i < TABLE_SIZE; i++) {
        wordList[i].word = malloc(MAX_SIZE);

        if (wordList[i].word == NULL) {
            printf("Malloc failed on %d iteration.\n", i); // Reporting error
            exit (-1); // Exiting program due to error occuring
        }

        wordList[i].count = 0; // Initialize count to 0
    }
    
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    // Declare array of pthread_t to identify threads
    pthread_t thread_id[threadCount];

    // Creating number of threads specified by command line argument
    for (int i = 0; i < threadCount; i++) {

        // Exits program if thread creation fails - Returns 0 if successful
        err = pthread_create(&thread_id[i], NULL, runner, (void*)NULL);
        if (err != 0) {
            printf("Creation of pthread #%d failed.\n", i);
            return(err);
        }
    }

    // // Joining number of threads specified by command line argument
    for (int i = 0; i < threadCount; i++) {

        // Exits program if pthread fails to terminate properly - Returns 0 if successful
        err = pthread_join(thread_id[i], NULL);
        if (err != 0) {
            printf("Joining of pthread #%d failed.\n", i);
            return(err);
        }
    }

    // ***TO DO *** Process TOP 10 and display
    wordInfo temp; // Temporary place holder for sorting algorithm

    // Bubble sort with place holder variable used to rearrange array from highest to lowest.
    for (int i = 0; i < currentWords; i++) {
        for(int j = i + 1; j < currentWords; j++) {
            if (wordList[i].count < wordList[j].count) {
                temp = wordList[i];
                wordList[i] = wordList[j];
                wordList[j] = temp;
            }
        }
    }

    printf("\n\n\n");
    printf("Word Frequency Count on %s with %d threads\n", argv[1], threadCount);
    printf("Printing top 10 words 6 characters or more.\n");

    // Output top 10 words in format shown in assignment writeup
    for (int i = 0; i < 10; i++) {
        printf("Number %d is %s with a count of %d\n", i+1, wordList[i].word, wordList[i].count);
    }

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************


    // ***TO DO *** cleanup

    // Closing file - Check if any process is pointing to it
    // err is returned if close fails - '0' indicates success
    err = close(fileDescriptor);
    if (err != 0) {
        printf("Close has failed. Error #%d\n", err);
        return(err);
    }

    // Only the word variable from the structure was malloc'ed memory so it must be freed
    for (int i = 0; i < TABLE_SIZE; i++) {
        free(wordList[i].word); // For every malloc, there shall be a corresponding free
        wordList[i].word = NULL; // Defensive - Protect from dangling pointer
    }


    // Destroy mutex upon completion of program
    // err is return if pthread_mutex_destory fails - '0' indicates success
    err = pthread_mutex_destroy(&criticalLock);
    if (err != 0) {
        printf("pthread destory has failed. Error #%d\n", err);
        return err;
    }

    return 0;
}
