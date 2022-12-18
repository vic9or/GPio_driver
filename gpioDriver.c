#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define DRIVER_NAME "gpiodriver"
#define DRIVER_CLASS "gpiodriverclass"

MODULE_AUTHOR("VICTOR");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A device driver for the raspberry pi");

static dev_t deviceNum;
static struct class *deviceClass;
static struct cdev deviceDriver;
static int driverOpen(struct inode *device_file, struct file *instance){
	printk("Open as called\n");
	return 0;
}

static int driverRelease(struct inode *device_file, struct file *instance){
	printk("Release was called\n");
	return 0;
}

static ssize_t driverRead(struct file *File, char *user_buffer, size_t size, loff_t *offs){
	int to_read, not_read;
	char temp[3] = " \n";

	to_read = min(size, sizeof(temp));

	temp[0] = gpio_get_value(17) + '0';

	not_read = copy_to_user(user_buffer, &temp, to_read);

	return to_read - not_read;
}

static ssize_t driverWrite(struct file *File, const char *user_buffer, size_t count, loff_t *offs){
	int to_read, not_read;
	char value;

	to_read = min(count, sizeof(value));

	not_read = copy_from_user(&value, user_buffer, to_read);

	switch(value){
		case '0':
			gpio_set_value(4,0);
			break;
		case '1':
			gpio_set_value(4,1);
		default:
			printk("Invalid Input!\n");
			break;
	}
	return to_read - not_read;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driverOpen,
	.release = driverRelease,
	.read = driverRead,
	.write = driverWrite
};

static int __init moduleInit(void){
	printk("Module is inserted\n");

	if (alloc_chrdev_region(&deviceNum, 0, 1, DRIVER_NAME) < 0){
		printk("Device Number could not be allocated\n");
		return -1;
	}

	if ((deviceClass = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
		printk("Device class could not be created\n");
		goto ClassError;
	}

	if (device_create(deviceClass, NULL, deviceNum, NULL, DRIVER_NAME) == NULL){
		printk("Can not create device File\n");
		goto FileError;
	}

	cdev_init(&deviceDriver, &fops);

	if(cdev_add(&deviceDriver, deviceNum, 1) == -1){
		printk("Could not register device");
		goto AddError;
	}

	if(gpio_request(4, "rpi-gpio-4")){
		printk("Can not allocate GPIO 4\n");
		goto AddError;
	}

	if(gpio_direction_output(4,0)){
		printk("Can not set gpio 4 as output\n");
		goto Gpio4Error;
	}

	if(gpio_request(17, "rpi-gpio-17")){
		printk("Can not allocate GPIO 17\n");
		goto Gpio4Error;
	}

	if(gpio_direction_input(17)){
		printk("Can not set gpio 17 as input\n");
		goto Gpio17Error;
	}
	return 0;
Gpio17Error:
	gpio_free(17);
Gpio4Error:
	gpio_free(4);
AddError:
	device_destroy(deviceClass, deviceNum);
FileError:
	class_destroy(deviceClass);
ClassError:
	unregister_chrdev_region(deviceNum, 1);
	return -1;
}

static void __exit moduleExit(void){
	gpio_set_value(4,0);
	gpio_free(17);
	gpio_free(4);
	cdev_del(&deviceDriver);
	device_destroy(deviceClass,deviceNum);
	class_destroy(deviceClass);
	unregister_chrdev_region(deviceNum, 1);	
	printk("Module is released\n");

}

module_init(moduleInit);
module_exit(moduleExit);
