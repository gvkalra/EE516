#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

static int
stack_dev_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t
stack_dev_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	return 0;
}

static ssize_t
stack_dev_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	return 0;
}

static int
stack_dev_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int __init
stack_dev_init(void)
{
	printk(KERN_INFO "Hello World! \n");
	return 0;
}

static void __exit
stack_dev_exit(void)
{
	printk(KERN_INFO "Goodbye World! \n");
}

static struct file_operations fops = {
	.open = stack_dev_open,
	.read = stack_dev_read,
	.write = stack_dev_write,
	.release = stack_dev_release,
};

module_init(stack_dev_init);
module_exit(stack_dev_exit);

MODULE_AUTHOR("Gaurav Kalra");
MODULE_DESCRIPTION("HW01 device driver providing a stack");
MODULE_LICENSE("GPL");
