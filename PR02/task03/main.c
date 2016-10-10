#include "utils.h"

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/mm_types.h>

#define PROC_NAME "proc_memory"

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
		struct mm_struct *active_mm;
	};

	struct list_head {
		 struct list_head *next, *prev;
	};

	struct mm_struct {
		...
		unsigned long total_vm;
		...
		struct mm_rss_stat rss_stat;
	};

	struct mm_rss_stat {
		atomic_long_t count[NR_MM_COUNTERS];
	};

	 enum {
		MM_FILEPAGES,  //Resident file mapping pages (Type of File Mapped Page)
		MM_ANONPAGES,  //Resident anonymous pages (Type of Anonymous Page - Stack, Heap)
		MM_SWAPENTS,   //Anonymous swap entries
		MM_SHMEMPAGES, //Resident shared memory pages
		NR_MM_COUNTERS
	};

	MM_FILEPAGES + MM_ANONPAGES = RSS Memory
	total_vm = VIRT Memory
*/

static int
pl_show(struct seq_file *m, void *v)
{
	struct task_struct *tsk;
	char name[TASK_COMM_LEN];
	unsigned long long virt = 0;
	long long rss = 0;

	/* header */
	seq_printf(m, "PID       ProcessName         "
		"VIRT(KB)            RSS Mem(KB)         \n");

	/* Print name, PID, VIRT & RSS of each process */
	for_each_process(tsk) {
		virt = rss = 0;

		/* It is possible for active_mm to be NULL.
		 * In this case, we simply assume VIRT and RSS to be 0.
		 * Ref: https://www.kernel.org/doc/Documentation/vm/active_mm.txt
		 */
		if (tsk->active_mm != NULL) {
			/* VIRT memory */
			virt = tsk->active_mm->total_vm;
			virt *= (PAGE_SIZE >> 10); /* convert pages to KB */

			/* RSS Memory */
			rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_FILEPAGES]);
			rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_ANONPAGES]);
			rss *= (PAGE_SIZE >> 10); /* convert pages to KB */
		}

		/* print information */
		seq_printf(m, "%-10u%-20s%-20llu%-20llu\n",
			task_pid_nr(tsk),
			get_task_comm(name, tsk),
			virt, rss);
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

	/* create /proc/proc_memory */
	pl = proc_create(PROC_NAME, 0, NULL, &fops);
	if (pl == NULL) {
		err("Failed to create proc_memory");
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
MODULE_DESCRIPTION("PR02 per Process Memory Usage");
MODULE_LICENSE("GPL");