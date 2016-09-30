#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

#include "utils.h"

/*
struct task_struct {
	...
	pid_t pid;
	...
	struct list_head tasks;
	...
	char comm[TASK_COMM_LEN];
};

struct list_head {
	 struct list_head *next, *prev;
};
*/

static void *
pl_seq_start(struct seq_file *m, loff_t *pos)
{
	/* new sequence */
	if (*pos == 0) {
		return &init_task;
	}
	/* sequence end */
	else {
		*pos = 0;
		return NULL;
	}
}

static void *
pl_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct task_struct *n_tsk, *c_tsk;

	c_tsk = v;

	if ((n_tsk = next_task(c_tsk)) != &init_task)
		return n_tsk;

	return NULL;
}

static void
pl_seq_stop(struct seq_file *m, void *v)
{
}

static int
pl_seq_show(struct seq_file *m, void *v)
{
	struct task_struct *tsk = v;
	char buf[TASK_COMM_LEN];

	seq_printf(m, "%s %u\n",
		get_task_comm(buf, tsk), task_pid_nr(tsk));

	return 0;
}

static struct seq_operations sops = {
	/* sets the iterator up and returns the first element of sequence */
	.start = pl_seq_start,

	/* returns the next element of sequence */
	.next = pl_seq_next,

	/* shuts it down */
	.stop = pl_seq_stop,

	/* prints element into the buffer */
	.show = pl_seq_show,
};

static int
pl_open(struct inode *inode, struct file *file)
{
	/* initialize sequential file
	 * Ref: https://www.kernel.org/doc/htmldocs/filesystems/API-seq-open.html
	*/
	return seq_open(file, &sops);
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = pl_open,

	/* read method for sequential files */
	.read = seq_read,

	/* llseek method for sequential files */
	.llseek = seq_lseek,

	/* free the structures associated with sequential file */
	.release = seq_release,
};

static void
_pl_module_exit(void)
{
	dbg("");
}

static int __init
pl_module_init(void)
{
	struct proc_dir_entry *pl;

	dbg("");

	pl = proc_create("proc_list", 0, NULL, &fops);
	if (pl == NULL) {
		err("Failed to create proc_list");
		goto error;
	}
	return 0;

error:
	_pl_module_exit();
	return -1;
}

static void __exit
pl_module_exit(void)
{
	_pl_module_exit();
}

module_init(pl_module_init);
module_exit(pl_module_exit);

MODULE_AUTHOR("Gaurav Kalra");
MODULE_DESCRIPTION("PR02 Traverse Process - tasklist");
MODULE_LICENSE("GPL");