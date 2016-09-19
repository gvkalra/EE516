#include "stack.h"
#include "utils.h"

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define CHARDEV_NAME "stack_device"
#define CHARDEV_NR_MINOR 0
#define CHARDEV_NR_DEVICES 1

static dev_t dev_first = 0;
static struct cdev *cdev = NULL;
static struct class *class = NULL;
struct device *device = NULL;

static int
stack_dev_open(struct inode *inode, struct file *file)
{
	dbg("");
	return 0;
}

static ssize_t
stack_dev_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
	int item, ret;

	dbg("");

	ret = st_pop(&item);
	if (ret < 0)
		return -ESPIPE; /* Illegal seek */

	if (copy_to_user(buffer, &item, sizeof(item)))
		return -EFAULT; /* Bad address */

	return 0;
}

static ssize_t
stack_dev_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
	int item, ret;

	dbg("");

	if (copy_from_user(&item, buffer, sizeof(item)))
		return -EFAULT; /* Bad address */
	
	ret = st_push(item);
	if (ret < 0)
		return -ENOMEM; /* Out of memory */

	return 0;
}

static int
stack_dev_release(struct inode *inode, struct file *file)
{
	dbg("");

	st_clean();

	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = stack_dev_open, /* open() */
	.read = stack_dev_read, /* read() */
	.write = stack_dev_write, /* write() */
	.release = stack_dev_release, /* close() */
};

static void
_stack_dev_exit(void)
{
	dbg("");

	if (device != NULL) {
		device_destroy(class, dev_first);
		device = NULL;
	}

	if (class != NULL) {
		class_destroy(class);
		class = NULL;
	}

	if (cdev != NULL) {
		cdev_del(cdev);
		cdev = NULL;
	}

	if (dev_first != 0) {
		unregister_chrdev_region(dev_first, CHARDEV_NR_DEVICES);
		dev_first = 0;
	}

	info("[STACK_DEVICE] released");
}

static int __init
stack_dev_init(void)
{
	int ret;

	dbg("");

	ret = alloc_chrdev_region(&dev_first, CHARDEV_NR_MINOR, CHARDEV_NR_DEVICES, CHARDEV_NAME);
	if (ret < 0) {
		err("Failed allocating device number");
		goto error;
	}

	cdev = cdev_alloc();
	if (cdev == NULL) {
		err("Failed allocating character device");
		goto error;
	}
	cdev->owner = THIS_MODULE;
	cdev->ops = &fops;

	ret = cdev_add(cdev, dev_first, CHARDEV_NR_DEVICES);
	if (ret < 0) {
		err("Failed adding character device to the system");
		goto error;
	}

	info("[STACK_DEVICE] allocated Major(%d) and Minor(%d)", MAJOR(dev_first), MINOR(dev_first));

	/* Create /dev/stack_device node */
	class = class_create(THIS_MODULE, CHARDEV_NAME);
	if (class == NULL) {
		err("Failed creating device class");
		goto error;
	}
	device = device_create(class, NULL, dev_first, NULL, "%s", CHARDEV_NAME);
	if (device == NULL) {
		err("Failed creating /dev node");
		goto error;
	}

	return 0;

error:
	_stack_dev_exit();
	return -1;
}

static void __exit
stack_dev_exit(void)
{
	_stack_dev_exit();
}

module_init(stack_dev_init);
module_exit(stack_dev_exit);

MODULE_AUTHOR("Gaurav Kalra");
MODULE_DESCRIPTION("HW01 device driver providing a stack");
MODULE_LICENSE("GPL");