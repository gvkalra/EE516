#include "sequence.h"
#include "utils.h"
#include "manager.h"
#include "sorting.h"

#include <linux/sched.h>
#include <linux/task_io_accounting_ops.h>

static void *
pl_seq_start(struct seq_file *m, loff_t *pos)
{
	dbg("");

	/* new sequence, return init_task */
	if (*pos == 0) {
		manager_init();
		return &init_task;
	}
	/* sequence end, terminate */
	else {
		*pos = 0;

		manager_show_monitor(m, get_current_sort_order());
		manager_release();

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
	unsigned long virt = 0;
	long rss = 0;
	struct task_io_accounting acct = tsk->ioac; /* initialize accounting data */

	dbg("");

	/* virt & rss */
	if (tsk->active_mm != NULL) {
		virt = tsk->active_mm->total_vm;
		rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_FILEPAGES]);
		rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_ANONPAGES]);
	}

	/* account each thread */
	t = tsk;
	task_io_accounting_add(&acct, &tsk->signal->ioac);
	while_each_thread(tsk, t)
		task_io_accounting_add(&acct, &t->ioac);

	/* send to manager */
	manager_add_entry(task_pid_nr(tsk), get_task_comm(buf, tsk),
		virt, rss,
		acct.read_bytes, acct.write_bytes,
		(acct.read_bytes + acct.write_bytes));

	/* print information */
	dbg("%s [PID: %u]\n"
		"\tvirt: %lu\n"
		"\trss: %lu\n"
		"\tread_bytes: %llu\n"
		"\twrite_bytes: %llu\n",
		get_task_comm(buf, tsk),
		task_pid_nr(tsk),
		virt,
		rss,
		(unsigned long long)acct.read_bytes,
		(unsigned long long)acct.write_bytes);

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