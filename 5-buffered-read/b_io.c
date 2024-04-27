/**************************************************************
* Class:  CSC-415-01 Spring 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub UserID: gojilikeog
* Project: Assignment 5 – Buffered I/O
*
* File: b_io.c
*
* Description: This program is to write a program that handles
*              the buffering instead of standard Linux functions.
*              To do so, we must implement a set of routines similar
*              to the ones Linux performed (open, read, close) to
*              perform the operations required to load the text file
*              into a buffer, and copy the specified blocks into a
*              user provided buffer.
*
**************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLowSmall.h"

#define MAXFCBS 20	//The maximum number of files open at one time


// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb
	{
	fileInfo * fi;	//holds the low level systems file info

	// Add any other needed variables here to track the individual open file
	int currentPosition; // Tracked buffer position
	uint64_t lbaResult; // Catch what LBAread returns - return value is unit64_t
	char * fcbBuffer; // Buffer access outside of declared function - b_open

	// Track buffer - based on assignment #2
	int textLeft; // Text left to copy from block
	int totalTextLeft; // text left to commit compared to total count
	int activatedBuffer; // Used once to signal empty buffer for inital read

	} b_fcb;
	
//static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;	

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init ()
	{
	if (startup)
		return;			//already initialized

	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].fi = NULL; //indicates a free fcbArray

		fcbArray[i].textLeft = 0; // Positioned here to reset value to zero if reused
		fcbArray[i].currentPosition = 0; // Positioned here to reset value to zero if reused

		}
		
	startup = 1;
	}

//Method to get a free File Control Block FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].fi == NULL)
			{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;		//Not thread safe but okay for this project
			}
		}

	return (-1);  //all in use
	}

// b_open is called by the "user application" to open a file.  This routine is 
// similar to the Linux open function.  	
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.  
// For this assignment the flags will be read only and can be ignored.

b_io_fd b_open (char * filename, int flags)
	{
	if (startup == 0) b_init();  //Initialize our system

	//*** TODO ***//  Write open function to return your file descriptor
	//				  You may want to allocate the buffer here as well
	//				  But make sure every file has its own buffer

	// This is where you are going to want to call GetFileInfo and b_getFCB

	// You will call GetFileInfo to find the filesize and location of the
	// desired file - per assignment specifications
	fileInfo* newFile = GetFileInfo(filename);

	// Error catching - if GetFileInfo returns NULL, return a negative number
	// else block is for TESTING ONLY - delete once not needed
	if (newFile->fileName == NULL) {
		return(-1);
	}

	// Unique identifiers obtained from b_getFCB - returns null if
	// no free FCB's avaliable
	b_io_fd fd = b_getFCB();

	//Error catching - Limit specified we can assume no more than 20 files open
	//				   at a time. If we exceed this, program has no FCB avaliable.
	//                 Abort if this is the case.
	if (fd == -1) {
		printf("No free FCB's avaliable. Aborting.\n");
		exit(fd);
	}

	// fileInfo pointer assigned to fcbArray entry with unique identifier given by b_getFCB()
	fcbArray[fd].fi = newFile;

	// Buffer used in b_read allocated in b_open to ensure memorry is allocated only for files
	// that are valid. +1 as a precaution
	char* bufferBlock = calloc(1, B_CHUNK_SIZE + 1);

	// Report failure of calloc if occured - Abort program
	// Calloc used to prevent conditional jump or move on uninitialized value in bufferBlock
	if (bufferBlock == NULL) {
		printf("Calloc failed for bufferBlock.\n");
		exit(-1);
	}

	fcbArray[fd].fcbBuffer = bufferBlock; // Pointer to buffer for individual fcb
	fcbArray[fd].totalTextLeft = fcbArray[fd].fi->fileSize; // Populate for use in b_read

	return fd; // Currently returns index of assigned File Control Block
	}



// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to 
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File	
int b_read (b_io_fd fd, char * buffer, int count)
	{
	//*** TODO ***//  
	// Write buffered read function to return the data and # bytes read
	// You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	// and check that the specified FCB is actually in use	
	if (fcbArray[fd].fi == NULL)		//File not open for this descriptor
		{
		return -1; 
		}	

	// Your Read code here - the only function you call to get data is LBAread.
	// Track which byte in the buffer you are at, and which block in the file	

	// ACTIVATION BLOCK FOR INITAL b_read
	if (fcbArray[fd].activatedBuffer == 0) {
		fcbArray[fd].activatedBuffer = 1;

		// Takes the fcb buffer, the number of blocks, and the block position of the fcb
		fcbArray[fd].lbaResult = LBAread(fcbArray[fd].fcbBuffer, 1, fcbArray[fd].fi->location);
		fcbArray[fd].currentPosition = 0;

		// Use of strlen to verify actual length of what was loaded into the buffer
		// What is actually in buffer could be less than B_CHUNK_SIZE
		fcbArray[fd].textLeft = strlen(fcbArray[fd].fcbBuffer);
	}

	// Catch case for end of file - If LBAread returns 0 or bytes left in file = 0
	if (fcbArray[fd].lbaResult == 0 | fcbArray[fd].totalTextLeft == 0) {
		return 0;
	}

	int actualCount = 0; // Variable to track what is actually copied into user buffer
	int tempLeft = count; // Variable for arithmetic to track how much is left to commit of count

	// First point of entry - Overflow detection
	//
	// Enters while loop if the text remaining in the fcb buffer is less than 
	// what is left to push to the user buffer
	while (fcbArray[fd].textLeft <= tempLeft) {

		// If the amount requested by the function is greater than the bytes left in the file,
		// reassign the textLeft variable to match the remaining bytes - Reuse existing code
		if (fcbArray[fd].textLeft > fcbArray[fd].totalTextLeft) {
			fcbArray[fd].textLeft = fcbArray[fd].totalTextLeft;
		}

		// Copy textLeft number of bytes into user buffer, starting from FCB buffer + currentPosition offset
		memcpy(buffer, fcbArray[fd].fcbBuffer + fcbArray[fd].currentPosition, fcbArray[fd].textLeft);

		actualCount = actualCount + fcbArray[fd].textLeft; // Increment with number of bytes copied
		tempLeft = tempLeft - fcbArray[fd].textLeft; // Deincrement with number of bytes copied

		// Deincrement total text size remaining with number of bytes copied
		fcbArray[fd].totalTextLeft = fcbArray[fd].totalTextLeft - fcbArray[fd].textLeft;

		// location is incremented to advance the position of the buffer copied in
		fcbArray[fd].fi->location = fcbArray[fd].fi->location + 1;
		fcbArray[fd].lbaResult = LBAread(fcbArray[fd].fcbBuffer, 1, fcbArray[fd].fi->location);

		// If LBAread returns a 0, it indicates there are no more blocks to read.
		// Return actualCount - count actually commited to user buffer from this point
		if (fcbArray[fd].lbaResult == 0 | fcbArray[fd].totalTextLeft <= 0) {
			return actualCount;
		}

		fcbArray[fd].currentPosition = 0; // Reset due to newly loaded buffer

		// Verify actual text length of newly loaded buffer - Could be less than B_CHUNK_SIZE
		// Positioned after in case no need to perform due to possible function ending flag.
		fcbArray[fd].textLeft = strlen(fcbArray[fd].fcbBuffer);
	}
	
	// Enters if statement if text remaining in fcb buffer is greater than what is left
	// to push into the user buffer - Can be entered after execution of while loop to
	// catch remaining bytes to copy to user buffer.
	if (fcbArray[fd].textLeft > tempLeft) {

		// Catch case - Amount left to push into user buffer requested by count is greater
		// than the amount of bytes left in the file - reassign tempLeft to reuse code
		if (tempLeft >= fcbArray[fd].totalTextLeft) {
			tempLeft = fcbArray[fd].totalTextLeft;
		}

		// tempLeft copied in due to no need to deincrement value as count amount of bytes
		// can be accomadated with this pass
		memcpy(buffer, fcbArray[fd].fcbBuffer + fcbArray[fd].currentPosition, tempLeft);

		actualCount = actualCount + tempLeft; // Increment with number of bytes copied
		// Deincrement total text size remaining with number of bytes copied
		fcbArray[fd].totalTextLeft = fcbArray[fd].totalTextLeft - tempLeft;
		fcbArray[fd].currentPosition = fcbArray[fd].currentPosition + tempLeft;

		// Deincrement text count left in fcb buffer
		fcbArray[fd].textLeft = fcbArray[fd].textLeft - tempLeft;
	}

	return actualCount;




	}
// b_close frees and allocated memory and places the file control block back 
// into the unused pool of file control blocks.
int b_close (b_io_fd fd)
	{
	//*** TODO ***//  Release any resources

	free(fcbArray[fd].fcbBuffer); // For every malloc, there shall be a corresponding free
	fcbArray[fd].fcbBuffer = NULL; // Defensive - Protect from dangling pointer

	fcbArray[fd].activatedBuffer = 0; // Reassigned flag back to 0 to be reused
	fcbArray[fd].fi = NULL; // Reassigned NULL to indicate free file control block

	return 0;
	}
	
