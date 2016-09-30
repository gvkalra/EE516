#include "sorting.h"
#include "utils.h"

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>

#define BUF_SIZE 512

static int sort_order = SORT_ORDER_PID;

static int
ps_show(struct seq_file *m, void *v)
{
	switch (sort_order) {
	case SORT_ORDER_VIRT:
		seq_printf(m, "PID \t [VIRT] \t RSS \t I/O\n");
		break;
	case SORT_ORDER_RSS:
		seq_printf(m, "PID \t VIRT \t [RSS] \t I/O\n");
		break;
	case SORT_ORDER_IO:
		seq_printf(m, "PID \t VIRT \t RSS \t [I/O]\n");
		break;
	case SORT_ORDER_PID:
	default:
		seq_printf(m, "[PID] \t VIRT \t RSS \t I/O\n");
		break;
	}

	return 0;
}

static int
ps_open(struct inode *inode, struct file *file)
{
	dbg("");
	return single_open(file, ps_show, NULL);
}

static ssize_t
ps_write(struct file *file, const char __user *user_buf, size_t length, loff_t *offset)
{
	char buf[BUF_SIZE];
	dbg("");

	memset(buf, 0x00, sizeof(buf));

	if (length > BUF_SIZE)
		length = BUF_SIZE;

	/* copy data from user space */
	if (copy_from_user(buf, user_buf, length))
		return -EFAULT; /* Bad address */

	/* null terminate buffer */
	buf[length - 1] = '\0';

	dbg("buf: [%s]", buf);

	if (strcmp(buf, "pid") == 0)
		sort_order = SORT_ORDER_PID;
	else if (strcmp(buf, "virt") == 0)
		sort_order = SORT_ORDER_VIRT;
	else if (strcmp(buf, "rss") == 0)
		sort_order = SORT_ORDER_RSS;
	else if (strcmp(buf, "io") == 0)
		sort_order = SORT_ORDER_IO;
	else
		info("Invalid value! sort_order is not changed");

	dbg("sort_order: [%d]", sort_order);

	/* copied from user space, return bytes */
	return length;
}

/* file operations */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = ps_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = ps_write,
	.release = single_release,
};

inline struct file_operations *
get_sorting_ops(void)
{
	return &fops;
}

inline int
get_current_sort_order(void)
{
	return sort_order;
}