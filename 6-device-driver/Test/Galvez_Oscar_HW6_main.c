/**************************************************************
* Class:  CSC-415-01 Spring 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub UserID: gojilikeog
* Project: Assignment 6 – Device Driver
*
* File: Galvez_Oscar_HW6_main.c
*
* Description: Device driver to run tests on ceasers.c
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/ceasers"
#define IOCTL_SETKEY 100
#define IOCTL_ENCRYPT 200
#define IOCTL_DECRPYT 300


int main(int argc, char *argv[])
{
    char testText[] = "This is a test";
    char encrpyted[sizeof(testText)];

    int err; // Error catch
    int fd = open(DEVICE_PATH, O_RDWR);

    // Error handling - if open failed
    if (fd < 0)
    {
        printf("Error with device path. Aborting at open.\n");
        return -1;
    }

    printf("Decrypting\n");

    // Testing decrpyting
    write(fd, testText, sizeof(testText));
    read(fd, encrpyted, sizeof(encrpyted));
    printf("Result of encrpytion: %s\n", encrpyted);

    err = ioctl(fd, IOCTL_DECRPYT, 0);

    // Error handling - if ioctl fails
    if (err < 0)
    {
        printf("Error with ioctl. Aborting.");
        return -1;
    }

    char decrypted[sizeof(encrpyted)];
    // write(fd, encrpyted, sizeof(encrpyted));
    read (fd, decrypted, sizeof(encrpyted));
    printf("Result of decryption: %s\n", decrypted);

    close(fd);
    return 0;
}