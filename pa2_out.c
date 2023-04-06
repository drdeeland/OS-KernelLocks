/**
  File: pa2_out.c
  Class: COP4600-SP23
*/

#include <linux/module.h>	 	// Core header for modules.
#include <linux/device.h>		// Supports driver model.
#include <linux/kernel.h>	 	// Kernel header for convenient functions.
#include <linux/fs.h>		 	// File-system support.
#include <linux/uaccess.h>	  	// User access copy function support.
#include <linux/mutex.h>        // Linux mutex.
#define DEVICE_NAME "pa2_out" 		// Device name.
#define CLASS_NAME "char2"		///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");						 	///< The license type -- this affects available functionality
MODULE_AUTHOR("Dillon Flaschner, Guan-Yu Lin, and Anh-Dung Van");	///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkm_pa2-out Kernel Module"); 				///< The description -- see modinfo
MODULE_VERSION("0.1");						 	///< A version number to inform users
/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *lkmpa2outClass = NULL;		///< The device-driver class struct pointer
static struct device *lkmpa2outDevice = NULL; 		///< The device-driver device struct pointer

// Allocate 1KB memory to store the device message.
#define BUFFER_LENGTH 1024
static char message[BUFFER_LENGTH] = {0};
static int size_of_message = 0;				//last_index of message

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.read = read,
};

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "lkm_pa2-out: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "lkm_pa2-out could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "lkm_pa2-out: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmpa2outClass = class_create(THIS_MODULE, CLASS_NAME);
	// Check for error and clean up if there is
	if (IS_ERR(lkmpa2outClass))
	{
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(lkmpa2outClass); 					// Correct way to return an error on a pointer
	}
	printk(KERN_INFO "lkm_pa2-out: device class registered correctly\n");

	// Register the device driver
	lkmpa2outDevice = device_create(lkmpa2outClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmpa2outDevice))
	{								 	// Clean up if there is an error
		class_destroy(lkmpa2outClass); 					// Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(lkmpa2outDevice);
	}
	printk(KERN_INFO "lkm_pa2-out: device class created correctly\n");		// Made it! device was initialized

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "lkm_pa2-out: removing module.\n");
	device_destroy(lkmpa2outClass, MKDEV(major_number, 0)); 			// remove the device
	class_unregister(lkmpa2outClass);						// unregister the device class
	class_destroy(lkmpa2outClass);						// remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  		// unregister the major number
	printk(KERN_INFO "lkm_pa2-out: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "lkm_pa2-out: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "lkm_pa2-out: device closed.\n");
	return 0;
}

/*
 * Reads from device, displays in userspace, and deletes the read data
 */
static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error = 0, i;

	printk(KERN_INFO "lkm_pa2-out: Read stub");
	
	error = copy_to_user(buffer, message, size_of_message);
	
	if (error == 0) {
		printk(KERN_INFO "lkm_pa2-out: Sent %d characters to the user\n", size_of_message);
		for(i = 0; i < size_of_message; i++) {
			message[i] = '\0';
		}
		size_of_message = 0;
		return 0;
	}
	else {
		printk(KERN_INFO "lkm_pa2-out: Failed to send %d characters to the user\n", size_of_message);
		return -EFAULT;
	}
	
	return 0;
}