#include "sort.h"
#include "utils.h"

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>

#define BUF_SIZE 512
#define PROC_SORT "procmon_sorting"

/* Assume default sort order as PID */
static int sort_order = SORT_ORDER_PID;
static struct proc_dir_entry *ps;

static int
sort_show(struct seq_file *m, void *v)
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
	default: /* fallthrough */
		seq_printf(m, "[PID] \t VIRT \t RSS \t I/O\n");
		break;
	}

	return 0;
}

static int
sort_open(struct inode *inode, struct file *file)
{
	dbg("");
	return single_open(file, sort_show, NULL);
}

static ssize_t
sort_write(struct file *file, const char __user *user_buf, size_t length, loff_t *offset)
{
	char buf[BUF_SIZE];
	dbg("");

	/* clear buffer */
	memset(buf, 0x00, sizeof(buf));

	/* resize */
	if (length > BUF_SIZE)
		length = BUF_SIZE;

	/* copy data from user space */
	if (copy_from_user(buf, user_buf, length))
		return -EFAULT; /* Bad address */

	/* null terminate buffer */
	buf[length - 1] = '\0';

	dbg("buf: [%s]", buf);

	/* set sorting order */
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
static struct file_operations sort_ops = {
	.owner = THIS_MODULE,
	.open = sort_open,

	/* read method for sequential files */
	.read = seq_read,

	/* llseek method for sequential files */
	.llseek = seq_lseek,

	/* write() system call on VFS */
	.write = sort_write,

	/* free the structures associated with sequential file */
	.release = single_release,
};

void
sort_module_exit(void)
{
	dbg("");

	if (ps != NULL)
		proc_remove(ps);
}

int
sort_module_init(void)
{
	dbg("");

	/* create /proc/procmon_sorting */
	ps = proc_create(PROC_SORT, 0, NULL, &sort_ops);
	if (ps == NULL) {
		err("Failed to create procmon_sorting");
		goto error;
	}
	return 0;

error:
	sort_module_exit();
	return -1;
}

inline int
sort_get_order(void)
{
	return sort_order;
}