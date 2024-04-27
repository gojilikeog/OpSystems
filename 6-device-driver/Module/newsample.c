#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>

#include <linux/sched.h>
#define MY_MAJOR 415
#define MY_MINOR 0
#define DEVICE_NAME "newsample"

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

// Redirect is a write function (hi > to file)
static ssize_t myWrite (struct file * fs, const char __user * buf, size_t hsize, loff_t * off)
{
    struct myds * ds;
    ds = (struct myds *) fs->private_data; // Pointer
    ds->count = ds->count + 1;
    printk(KERN_INFO "We wrote : %lu on write number %d\n", hsize, ds->count);
    return hsize;
}

// using the 'cat' command is a read
static ssize_t myRead (struct file * fs, char __user * buf, size_t hsize, loff_t * off)
{
    struct myds * ds;
    ds = (struct myds *) fs->private_data; // Pointer
    ds->count = ds->count + 1;
    printk(KERN_INFO "We read : %lu on read/Write number %d\n", hsize, ds->count);
    return 0;
}

static int myOpen(struct inode * inode, struct file * fs)
{
    struct myds * ds;
    ds = vmalloc(sizeof(struct myds)); // Kernel memory allocation

    if (ds == 0) {
        printk(KERN_ERR "Can not vmalloc, File not opened.\n");
        return -1;
    }

    ds->count = 0;
    fs->private_data = ds; // Private_data is for the driver to use ONLY
    return 0;
}

static int myClose(struct inode * inode, struct file * fs) 
{
    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    vfree(ds);
    return 0;
}

// Gives us a backdoor to the file - Another way to do things
// Way to deal with device files where there may not be read/write
static long myIoCtl(struct file * fs, unsigned int command, unsigned long data)
{
    int * count;
    struct myds * ds;
    ds = (struct myds *) fs->private_data;

    if (command != 3)
    {
        printk(KERN_ERR "failed in myioctl.\n");
        return -1;
    }

    count = (int *) data;
    *count = ds->count;
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
    printk(KERN_INFO "Welcome - NULL Driver is loaded\n");

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

    printk(KERN_INFO "Goodbye from Sample NULL Driver!\n");
}