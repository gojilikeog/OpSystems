/**************************************************************
* Class:  CSC-415-01 Spring 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub UserID: gojilikeog
* Project: Assignment 6 – Device Driver
*
* File: ceasers.c
*
* Description: Device driver that, by default, will encrypt the
*              the sentence passed into it. Using MyIoCtl function,
*              we can designate the driver to unencrypt the stored
*              message, as well as set it back to encrypting and
*              setting the key to encrypt by.
**************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>

#include <linux/sched.h>
#define MY_MAJOR 415
#define MY_MINOR 0
#define DEVICE_NAME "ceasers"

#define BUFFER_LEN 128
#define IOCTL_SETKEY 100
#define IOCTL_ENCRYPT 200
#define IOCTL_DECRPYT 300

int major, minor;
char *kernel_buffer;

struct cdev my_cdev;
int actual_rx_size = 0;

MODULE_AUTHOR("Oscar Galvez");
MODULE_DESCRIPTION("Ceaser's Cypher");
MODULE_LICENSE("GPL");

struct myds {
    int count;
} myds;

// Static global variables used to not clash with established glboal variables in kernel
static int tempLength;

static char encrpytedMessage[BUFFER_LEN];
static char resultMessage[BUFFER_LEN];

static unsigned int encryptKey;
static int encryptFlag;

static int flag = 1;

struct user_data {
    int key;
    char message[BUFFER_LEN];
    int switchEncrypt;
    int length;
};

// Redirect is a write function (hi > to file)
static ssize_t myWrite (struct file * fs, const char __user * buf, size_t hsize, loff_t * off)
{
    struct user_data * ds;
    int ret;
    int i;
    char tempMessage[BUFFER_LEN];
    ds = (struct user_data *) fs->private_data; // Pointer
    printk(KERN_INFO "We are inside myWrite now\n");

    ret = copy_from_user(tempMessage, buf, hsize);
    tempLength = hsize;
    tempMessage[hsize - 1] = '\0';

    if (ret != 0)
    {
        printk(KERN_ERR "Not able to copy all data passed in. Abort in myWrite.\n");
        return -1;
    }

    printk("This was passed in to the global variable: %s length: %d\n", tempMessage, tempLength);

// Encrpytion algo
     for (i = 0 ; i < hsize && tempMessage[i]!= '\0'; i++)
     {
         if (tempMessage[i] >= 'a' && tempMessage[i] <= 'z' || tempMessage[i] >= 'A' && tempMessage[i] <= 'Z')
         if ((tempMessage[i] >= 'a' && tempMessage[i] <= 'z') || (tempMessage[i] >= 'A' && tempMessage[i] <= 'Z'))
         {
             encrpytedMessage[i] = ('a' + (tempMessage[i] - 'a' + 26 - encryptKey) % 26);
             encrpytedMessage[i] = ('a' + (tempMessage[i] - 'a' + 26 - encryptKey) % 26);
         } else
         {
             encrpytedMessage[i] = tempMessage[i];
         }
     }

    return hsize;
}

// using the 'cat' command is a read
static ssize_t myRead (struct file * fs, char __user * buf, size_t hsize, loff_t * off)
{
    if (flag == 0) {
        return 0;
    }

    struct user_data * ds;
    int ret;
    int i;
    printk(KERN_INFO "We are inside myRead now.\n");
    ds = (struct user_data *) fs->private_data; // Pointer

    printk(KERN_INFO "Testing struct variable: %d\n", tempLength);
    //printk(KERN_INFO "In the buffer: %s\n", tempMessage);

    if (encryptFlag == 1) {
        ret = copy_to_user(buf, encrpytedMessage, tempLength);

        if (ret != 0)
        {
            printk(KERN_ERR "Not able to copy all the data passed in. Abort in myRead\n");
            return -1;
        }
    }

    printk(KERN_INFO "TEST: Encrypt mesg: %s\n", encrpytedMessage);


    for (i = 0 ; i < hsize && encrpytedMessage[i]!= '\0'; i++) // Decryption algorithm
         {
             if ((encrpytedMessage[i] >= 'a' && encrpytedMessage[i] <= 'z') || (encrpytedMessage[i] >= 'A' && encrpytedMessage[i] <= 'Z'))
             {
                 resultMessage[i] = ('a' + (encrpytedMessage[i] - 'a' + 26 + encryptKey) % 26);
             } else
             {
                 resultMessage[i] = encrpytedMessage[i];
             }
         }

         printk(KERN_INFO "Test: Decrypt mesg: %s\n", resultMessage);

        ret = copy_to_user(buf, resultMessage, tempLength);

        if (ret != 0)
        {
            printk(KERN_ERR "Not able to copy all the data passed in. Abort in myRead\n");
            return -1;
        }
    
    flag = 0;
    return tempLength;
}

static int myOpen(struct inode * inode, struct file * fs)
{
    struct user_data * ds;
    printk(KERN_INFO "Inside myOpen\n");
    ds = vmalloc(sizeof(struct user_data)); // Kernel memory allocation

    if (ds == 0) {
        printk(KERN_ERR "Can not vmalloc, File not opened.\n");
        return -1;
    }

    // Initalize variables contained within the struct
    ds->key = 0;
    ds->switchEncrypt = 0;

    encryptFlag = 1; // Flag to indicate to encrypt incoming messages
    encryptKey = 5; // Set default value for use in the cypher

    fs->private_data = ds; // Private_data is for the driver to use ONLY
    return 0;
}

static int myClose(struct inode * inode, struct file * fs) 
{
    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    vfree(ds); // For every vmalloc, there shall be a corresponding free
    return 0;
}

// Gives us a backdoor to the file - Another way to do things
// Way to deal with device files where there may not be read/write
static long myIoCtl(struct file * fs, unsigned int command, unsigned long data)
{
    int * count;

    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    count = (int *) data;

    printk(KERN_INFO "We are inside myIoCtl.\n");

    if (command == 2)
    {
        printk(KERN_ERR "failed in myioctl.\n");
        return -1;
    } else if (command == IOCTL_SETKEY)
    {
        printk(KERN_INFO "Setting cypher key to %ld\n", data);
        encryptKey = *count;
    } else if (command == IOCTL_DECRPYT)
    {
        printk(KERN_INFO "Setting cypher to decrypt.\n");
        encryptFlag = 0;
    } else if (command == IOCTL_ENCRYPT)
    {
        printk(KERN_INFO "Setting cypher to encrypt.\n");
        encryptFlag = 1;
    }

    return 0;
}

struct file_operations fops = {
    .open = myOpen,
    .release = myClose,
    .write = myWrite,
    .read = myRead,
    .unlocked_ioctl = myIoCtl,
    .owner = THIS_MODULE,
};

// creates a device node in /dev, returns error if not made
int init_module(void)
{
    int result, registers;
    dev_t devno;

    devno = MKDEV(MY_MAJOR, MY_MINOR);

    registers = register_chrdev_region(devno, 1, DEVICE_NAME);
    printk(KERN_INFO "Register chardev suceeded 1: %d\n", registers);
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    result = cdev_add(&my_cdev, devno, 1);
    printk(KERN_INFO "Dev Add chardev suceeded 2: %d\n", result);
    printk(KERN_INFO "Welcome - ceasers Driver is loaded\n");

    if (result < 0) {
        printk(KERN_ERR "Register chardev failed : %d\n", result);
    }

    return result;
}

void cleanup_module(void)
{
    dev_t devno;
    devno = MKDEV(MY_MAJOR, MY_MINOR);
    unregister_chrdev_region(devno, 1);
    cdev_del(&my_cdev);

    printk(KERN_INFO "Goodbye from ceasers Driver!\n");
}