#include "stack.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

static struct cdev cdev_s;

static int
stack_dev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "stack_dev_open()\n");
	return 0;
}

static ssize_t
stack_dev_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	int item, ret;

	printk(KERN_INFO "stack_dev_read()\n");

	ret = st_pop(&item);
	if (ret < 0)
		return -ESPIPE; /* Illegal seek */

	if (copy_to_user(buffer, &item, sizeof(item)))
		return -EFAULT; /* Bad address */

	return 0;
}

static ssize_t
stack_dev_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	int item, ret;

	printk(KERN_INFO "stack_dev_write()\n");

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
	printk(KERN_INFO "stack_dev_release()\n");

	st_clean();

	return 0;
}

static struct file_operations fops = {
	.open = stack_dev_open,
	.read = stack_dev_read,
	.write = stack_dev_write,
	.release = stack_dev_release,
};

static int __init
stack_dev_init(void)
{
	dev_t dev = MKDEV(250, 0);

	printk(KERN_INFO "stack_dev_init()\n");

	register_chrdev(250, "Stack Driver", &fops);
	cdev_init(&cdev_s, &fops);
	cdev_add(&cdev_s, dev, 128);

	return 0;
}

static void __exit
stack_dev_exit(void)
{
	printk(KERN_INFO "stack_dev_exit()\n");

	cdev_del(&cdev_s);
	unregister_chrdev_region(MKDEV(250,0), 128);
}

module_init(stack_dev_init);
module_exit(stack_dev_exit);

MODULE_AUTHOR("Gaurav Kalra");
MODULE_DESCRIPTION("HW01 device driver providing a stack");
MODULE_LICENSE("GPL");