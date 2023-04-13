/**
  File: pa2_in.c
  Class: COP4600-SP23
*/

#include <linux/module.h>	 	// Core header for modules.
#include <linux/device.h>		// Supports driver model.
#include <linux/kernel.h>	 	// Kernel header for convenient functions.
#include <linux/fs.h>		 	// File-system support.
#include <linux/uaccess.h>	  	// User access copy function support.
#include <linux/mutex.h>        // Linux mutex.
#define DEVICE_NAME "pa2_in" 		// Device name.
#define CLASS_NAME "char"		///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");						 	///< The license type -- this affects available functionality
MODULE_AUTHOR("Dillon Flaschner, Guan-Yu Lin, and Anh-Dung Van");	///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("pa2-in Kernel Module"); 				///< The description -- see modinfo
MODULE_VERSION("0.1");						 	///< A version number to inform users
/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *lkmpa2inClass = NULL;		///< The device-driver class struct pointer
static struct device *lkmpa2inDevice = NULL; 		///< The device-driver device struct pointer

// Allocate 1KB memory to store the device message, and EXPORT the variables
#define BUFFER_LENGTH 1024
static char pa2_message[BUFFER_LENGTH] = {0};
static int pa2_size_of_message = 0;				//last_index of message

EXPORT_SYMBOL(pa2_message);
EXPORT_SYMBOL(pa2_size_of_message);

// Make the mutex lock and EXPORT it.
static struct mutex pa2_lock;

EXPORT_SYMBOL(pa2_lock);

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.write = write,
};

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "pa2-in: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "pa2-in could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "pa2-in: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmpa2inClass = class_create(THIS_MODULE, CLASS_NAME);
	// Check for error and clean up if there is
	if (IS_ERR(lkmpa2inClass))
	{
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(lkmpa2inClass); 					// Correct way to return an error on a pointer
	}
	printk(KERN_INFO "pa2-in: device class registered correctly\n");

	// Register the device driver
	lkmpa2inDevice = device_create(lkmpa2inClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmpa2inDevice))
	{								 	// Clean up if there is an error
		class_destroy(lkmpa2inClass); 					// Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(lkmpa2inDevice);
	}

	mutex_init(&pa2_lock);

	printk(KERN_INFO "pa2-in: device class created correctly\n");		// Made it! device was initialized

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "pa2-in: removing module.\n");
	device_destroy(lkmpa2inClass, MKDEV(major_number, 0)); 			// remove the device
	class_unregister(lkmpa2inClass);						// unregister the device class
	class_destroy(lkmpa2inClass);						// remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  		// unregister the major number
	printk(KERN_INFO "pa2-in: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "pa2-in: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "pa2-in: device closed.\n");
	return 0;
}

/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int i, j = 0, added = 0;

	printk(KERN_INFO "pa2-in: Write stub");

	// check if the lock is taken
	if (mutex_is_locked(&pa2_lock)) {
		printk(KERN_INFO "pa2-in: Waiting on lock");
	}

	// attempt to acuire the lock
	mutex_lock(&pa2_lock);
	printk(KERN_INFO "pa2-in: Lock Acquired\n");
	printk(KERN_INFO "pa2-in: Critical Section entered\n");

	if (len == 0) {
		// Release lock
		mutex_unlock(&pa2_lock);
		printk(KERN_INFO "pa2-in: Lock Released\n");

		return 0;
	}

	if (pa2_size_of_message >= BUFFER_LENGTH) {
		printk(KERN_INFO "pa2-in: Buffer full\n");
	}
	else if (len + pa2_size_of_message > BUFFER_LENGTH) {
		for(i = pa2_size_of_message; i < BUFFER_LENGTH; i++) {
			pa2_message[i] = buffer[j++];
		}

		added = BUFFER_LENGTH - pa2_size_of_message;
		pa2_size_of_message = BUFFER_LENGTH;

		printk(KERN_INFO "pa2-in: Not enough space in the buffer, dropping the rest\n");
		printk(KERN_INFO "pa2-in: Received %d characters from the user\n", added);
	}
	else {
		sprintf(&(pa2_message[pa2_size_of_message]), "%s", buffer);

		pa2_size_of_message += len;

		printk(KERN_INFO "pa2-in: Received %zu characters from the user\n", len);
	}

	// Release lock
	mutex_unlock(&pa2_lock);
	printk(KERN_INFO "pa2-in: Lock Released\n");

	return len;
}
