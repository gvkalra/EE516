//#include "stack.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

static struct cdev cdev_s;

///////////////////////////// Move to separate file ///////////////////////////////
#define STACK_SIZE 256

struct {
	int stack[STACK_SIZE];
	int top;
} st = {
	.top = -1,
};

int st_is_full(void)
{
	printk(KERN_INFO "st_is_full()\n");

	if (st.top >= STACK_SIZE - 1)
		return 1;
	return 0;
}

int st_is_empty(void)
{
	printk(KERN_INFO "st_is_empty()\n");

	if (st.top == -1)
		return 1;
	return 0;
}

int st_push(int item)
{
	/* return -1 if stack is already full */
	if (st_is_full())
		return -1;

	printk(KERN_INFO "st_push()\n");

	st.top++;
	st.stack[st.top] = item;
	return 0;
}

int st_top(int *item)
{
	/* invalid arguments or stack is empty */
	if (item == NULL || st_is_empty())
		return -1;

	printk(KERN_INFO "st_top()\n");

	*item = st.stack[st.top];
	return 0;
}

int st_pop(int *item)
{
	/* invalid arguments or stack is empty */
	if (item == NULL || st_is_empty())
		return -1;

	printk(KERN_INFO "st_pop()\n");

	*item = st.stack[st.top];
	st.top--;
	return 0;
}

int st_get_size(void)
{
	printk(KERN_INFO "st_get_size()\n");

	return 0;
}

void st_clean(void)
{
	printk(KERN_INFO "st_clean()\n");

	st.top = -1;
}
///////////////////////////// Move to separate file [END] ///////////////////////////////

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