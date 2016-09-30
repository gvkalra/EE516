#include "sequence.h"
#include "utils.h"

#include <linux/sched.h>
#include <linux/mm_types.h>

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

static void *
pl_seq_start(struct seq_file *m, loff_t *pos)
{
	dbg("");

	/* new sequence, return init_task */
	if (*pos == 0) {
		return &init_task;
	}
	/* sequence end, terminate */
	else {
		*pos = 0;
		return NULL;
	}
}

static void *
pl_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct task_struct *n_tsk, *c_tsk;
	dbg("");

	/* set current task */
	c_tsk = v;

	/* return next task */
	if ((n_tsk = next_task(c_tsk)) != &init_task)
		return n_tsk;

	/* if next task == init_task, terminate */
	return NULL;
}

static void
pl_seq_stop(struct seq_file *m, void *v)
{
	dbg("");
	return; /* nop */
}

static int
pl_seq_show(struct seq_file *m, void *v)
{
	struct task_struct *tsk = v;
	char buf[TASK_COMM_LEN];
	unsigned long virt = 0;
	long rss = 0;
	dbg("");

	if (tsk->active_mm != NULL) {
		virt = tsk->active_mm->total_vm;
		rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_FILEPAGES]);
		rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_ANONPAGES]);
	}

	/* print information */
	seq_printf(m, "%s %u : VIRT [%lu] RSS [%lu]\n",
		get_task_comm(buf, tsk),
		task_pid_nr(tsk),
		virt, rss);

	/* return success */
	return 0;
}

/* sequence operations */
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

inline struct seq_operations *
get_sequence_ops(void)
{
	return &sops;
}