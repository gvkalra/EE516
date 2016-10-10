#include "utils.h"

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/task_io_accounting_ops.h>

#define PROC_NAME "proc_io"

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
		...
		struct task_io_accounting ioac;
	};

	struct list_head {
		 struct list_head *next, *prev;
	};

	struct task_io_accounting {
		#ifdef CONFIG_TASK_XACCT
 			u64 rchar; //bytes read
 			u64 wchar; //bytes written
 			u64 syscr; //# of read syscalls
 			u64 syscw; //# of write syscalls
 		#endif

		#ifdef CONFIG_TASK_IO_ACCOUNTING
 			//The number of bytes which this task has caused to be read from storage.
			u64 read_bytes;

			//The number of bytes which this task has caused, or shall cause to be written to disk.
			u64 write_bytes;
 
			//A task can cause "negative" IO too.  If this task truncates some
 			//dirty pagecache, some IO which another task has been accounted for
 			//(in its write_bytes) will not be happening.  We _could_ just
 			//subtract that from the truncating task's write_bytes, but there is
 			//information loss in doing that.
 			u64 cancelled_write_bytes;
		#endif
	};
*/

static int
pl_show(struct seq_file *m, void *v)
{
	struct task_struct *tsk, *t;
	char name[TASK_COMM_LEN];
	struct task_io_accounting acct;

	/* header */
	seq_printf(m, "PID       ProcessName         "
		"rchar(B)            wchar(B)            "
		"syscr(#)            syscw(#)            "
		"read_bytes(B)       write_bytes(B)      "
		"cancelled_write_bytes(B)\n");

	/* Print name, PID, I/O stats of each process */
	for_each_process(tsk) {
		acct = tsk->ioac; /* initialize accounting data */

		/* account each thread
		 * Ref: https://github.com/torvalds/linux/blob/master/fs/proc/base.c
		 * Function: do_io_accounting()
		*/
		t = tsk;
		task_io_accounting_add(&acct, &tsk->signal->ioac);
		while_each_thread(tsk, t)
			task_io_accounting_add(&acct, &t->ioac);

		/* print information */
		seq_printf(m, "%-10u%-20s%-20llu%-20llu%-20llu%-20llu%-20llu%-20llu%-24llu\n",
			task_pid_nr(tsk),
			get_task_comm(name, tsk),
			(unsigned long long)acct.rchar,
			(unsigned long long)acct.wchar,
			(unsigned long long)acct.syscr,
			(unsigned long long)acct.syscw,
			(unsigned long long)acct.read_bytes,
			(unsigned long long)acct.write_bytes,
			(unsigned long long)acct.cancelled_write_bytes);
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

	/* create /proc/proc_io */
	pl = proc_create(PROC_NAME, 0, NULL, &fops);
	if (pl == NULL) {
		err("Failed to create proc_io");
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
MODULE_DESCRIPTION("PR02 per Process I/O Usage");
MODULE_LICENSE("GPL");