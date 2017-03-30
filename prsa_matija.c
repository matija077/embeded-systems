#include <linux/module.h>
//#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h> 
#define DEVICE_NAME "prsa_matijachar"
#define CLASS_NAME "prsa_matija"

MODULE_AUTHOR("Matija Prsa");
MODULE_DESCRIPTION("Homework example");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static DEFINE_MUTEX(prsa_matija_mutex);

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   messageReal[512] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  prsa_matijaClass  = NULL; ///< The device-driver class struct pointer
static struct device* prsa_matijaDevice = NULL; ///< The device-driver device struct pointer

//static char *message = "Default input";
//module_param(message, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//MODULE_PARM_DESC(message, "just an ordinary input");
//static int test = 1;
//module_param(test, int, 0);
//MODULE_PARM_DESC(test, "test variable integer");

/*module_param(majorNumber, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(majorNumber, "Stores the device number -- determined automatically");
module_param(size_of_message, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(size_of_message, " Used to remember the size of the string stored");
module_param(numberOpens, int,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(numberOpens, "Counts the number of times the device is opened");
*/

static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
	.read = dev_read,
	.write = dev_write,
	.open = dev_open,
	.release = dev_release,
};

static int __init prsa_matija_module(void)
{
	printk(KERN_INFO "Character device Prsa_matija begins.\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT "PMChar: failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "PMChar:  register correctly with a major number %d\n", majorNumber);

	prsa_matijaClass = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(prsa_matijaClass)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register class name \n");
		return PTR_ERR(prsa_matijaClass);
	}
	printk(KERN_INFO "PMChar:  device class registered correctly\n");

	prsa_matijaDevice = device_create(prsa_matijaClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(prsa_matijaDevice)){
		class_destroy(prsa_matijaClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create device\n");
		return PTR_ERR(prsa_matijaDevice);
	}
	printk(KERN_INFO "PMChar:  device class created correctly\n");
	//printk(KERN_INFO "Dobrodosao <%s>\n", message);
	//printk(KERN_INFO "test: %d\n", test);
	mutex_init(&prsa_matija_mutex);
	return 0;
}

static int dev_open(struct inode *inodep, struct file *filep)
{
/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
	if (!mutex_trylock(&prsa_matija_mutex)){
		printk(KERN_ALERT "PMChar: Device aleady in use\n");
		return -EBUSY;
	}
	printk(KERN_INFO "PMChar:  Device driver has been opened\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
	int errorCount = 0;
	printk(KERN_INFO "PMChar:  Sending <%s> to aplication\n", messageReal);
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	errorCount = copy_to_user(buffer, messageReal, size_of_message);

	if (errorCount == 0){
		printk(KERN_INFO, "PMChar:  Sent <%s> to the aplication\n", messageReal);
		return (size_of_message = 0); //clear position to start
	} else {
		printk(KERN_WARNING "PMChar:  Failed to send message to the aplication\n");
		return -EFAULT; // Failed -- return a bad address message (i.e. -14)
	}
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	sprintf(messageReal, "Dobrodosao %s", buffer);
	size_of_message = strlen(messageReal);
	printk(KERN_INFO "PMChar:  Recieve message <%s> from aplication\n", messageReal);
	return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&prsa_matija_mutex);
	printk(KERN_INFO "PMChar:  Device successfully closed\n");
	return 0;
}

static void __exit clean_module(void)
{
	mutex_destroy(&prsa_matija_mutex);
	//reverse order from init
	device_destroy(prsa_matijaClass, MKDEV(majorNumber, 0)); //remoe the device
	class_unregister(prsa_matijaClass); 			//unregister the device class
	class_destroy(prsa_matijaClass);			//remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);		//unregister the major number
	printk(KERN_INFO "PMChar:  Goodbye from PMChar\n");
}

module_init(prsa_matija_module);
module_exit(clean_module);

