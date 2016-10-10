#include "utils.h"

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>

#define PROC_NAME "proc_list"

static struct proc_dir_entry *pl;

/*
Organization of task information in kernel:

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

static int
pl_show(struct seq_file *m, void *v)
{
	struct task_struct *tsk;
	char name[TASK_COMM_LEN];

	/* header */
	seq_printf(m, "PID       ProcessName         \n");

	/* Print pid & name of each process */
	for_each_process(tsk) {

		/* print information */
		seq_printf(m, "%-10u%-20s\n",
			task_pid_nr(tsk),
			get_task_comm(name, tsk));
	}

	return 0;
}

static int
pl_open(struct inode *inode, struct file *file)
{
	dbg("");
	return single_open(file, pl_show, NULL);
}

/* file operations */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = pl_open,

	/* read method for sequential files */
	.read = seq_read,

	/* llseek method for sequential files */
	.llseek = seq_lseek,

	/* free the structures associated with sequential file */
	.release = single_release,
};

static void
_pl_module_exit(void)
{
	dbg("");

	if (pl != NULL)
		proc_remove(pl);
}

static int __init
pl_module_init(void)
{
	dbg("");

	/* create /proc/proc_list */
	pl = proc_create(PROC_NAME, 0, NULL, &fops);
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