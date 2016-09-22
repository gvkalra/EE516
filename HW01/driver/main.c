#include "stack.h"
#include "utils.h"

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define CHARDEV_NAME "stack_device" /* device name */
#define CHARDEV_NR_MINOR 0 /* starting minor number */
#define CHARDEV_NR_DEVICES 1 /* number of devices (only 1 is supported) */

/* dynamically allocated device number */
static dev_t dev_first = 0;

/* character device */
static struct cdev *cdev = NULL;

/* device class */
static struct class *class = NULL;

/* device node */
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

	/* EOF, return 0 */
	if (st_is_empty())
		return 0;

	/* pop */
	ret = st_pop(&item);
	if (ret < 0)
		return ret;

	/* copy data into user space */
	if (copy_to_user(buffer, &item, sizeof(item)))
		return -EFAULT; /* Bad address */

	/* popped & copied to user space, return bytes */
	return sizeof(item);
}

static ssize_t
stack_dev_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
	int item, ret;

	dbg("");

	/* No more space */
	if (st_is_full())
		return -ENOMEM;

	/* copy data from user space */
	if (copy_from_user(&item, buffer, sizeof(item)))
		return -EFAULT; /* Bad address */

	/* push */
	ret = st_push(item);
	if (ret < 0)
		return ret;

	/* pushed & copied from user space, return bytes */
	return sizeof(item);
}

static int
stack_dev_release(struct inode *inode, struct file *file)
{
	dbg("");
	return 0;
}

static long
stack_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
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
/*
 * References:
 * [1] https://github.com/torvalds/linux/blob/master/fs/ext4/file.c#L698
 * [2] https://github.com/torvalds/linux/blob/master/fs/ext4/ioctl.c#L436
*/
	.unlocked_ioctl = stack_dev_ioctl, /* unlocked_ioctl() */
};

static void
_stack_dev_exit(void)
{
	dbg("");

	/* Remove /dev/stack_device node */
	if (device != NULL) {
		device_destroy(class, dev_first);
		device = NULL;
	}

	/* Destroy device class */
	if (class != NULL) {
		class_destroy(class);
		class = NULL;
	}

	/* Remove cdev */
	if (cdev != NULL) {
		cdev_del(cdev);
		cdev = NULL;
	}

	/* Un-register char device number */
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

	/* Register char device number */
	ret = alloc_chrdev_region(&dev_first, CHARDEV_NR_MINOR, CHARDEV_NR_DEVICES, CHARDEV_NAME);
	if (ret < 0) {
		err("Failed allocating device number");
		goto error;
	}

	/* Allocate cdev structure */
	cdev = cdev_alloc();
	if (cdev == NULL) {
		err("Failed allocating character device");
		goto error;
	}
	cdev->owner = THIS_MODULE;
	cdev->ops = &fops;

	/* Add char device to the system */
	ret = cdev_add(cdev, dev_first, CHARDEV_NR_DEVICES);
	if (ret < 0) {
		err("Failed adding character device to the system");
		goto error;
	}

	info("[STACK_DEVICE] allocated Major(%d) and Minor(%d)", MAJOR(dev_first), MINOR(dev_first));

	/* Create /dev/stack_device node
	 * Ref: https://github.com/euspectre/kedr/blob/master/sources/examples/sample_target/cfake.c
	*/
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
	/* we can't call stack_dev_exit()
	   because it's segment is controlled by __exit modifier
	*/
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