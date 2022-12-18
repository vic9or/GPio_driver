#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_AUTHOR("VICTOR");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A device driver for the raspberry pi");

