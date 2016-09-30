#include "sequence.h"
#include "utils.h"

#include <linux/sched.h>
#include <linux/task_io_accounting_ops.h>

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
	struct task_struct *tsk = v, *t;
	char buf[TASK_COMM_LEN];
	struct task_io_accounting acct = tsk->ioac; /* initialize accounting data */

	dbg("");

	/* account each thread */
	t = tsk;
	task_io_accounting_add(&acct, &tsk->signal->ioac);
	while_each_thread(tsk, t)
		task_io_accounting_add(&acct, &t->ioac);

	/* print information */
	seq_printf(m, "%s [PID: %u]\n"
		"\trchar: %llu\n"
		"\twchar: %llu\n"
		"\tsyscr: %llu\n"
		"\tsyscw: %llu\n"
		"\tread_bytes: %llu\n"
		"\twrite_bytes: %llu\n"
		"\tcancelled_write_bytes: %llu\n\n",
		get_task_comm(buf, tsk),
		task_pid_nr(tsk),
		(unsigned long long)acct.rchar,
		(unsigned long long)acct.wchar,
		(unsigned long long)acct.syscr,
		(unsigned long long)acct.syscw,
		(unsigned long long)acct.read_bytes,
		(unsigned long long)acct.write_bytes,
		(unsigned long long)acct.cancelled_write_bytes);

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