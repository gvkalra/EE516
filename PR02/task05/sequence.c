#include "sequence.h"
#include "manager.h"
#include "utils.h"

static void *
pl_seq_start(struct seq_file *m, loff_t *pos)
{
	dbg("");

	/* new sequence, return manager_init_entry */
	if (*pos == 0) {
		return &manager_init_entry;
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
	struct manager_entry *n_entry, *c_entry;
	dbg("");

	/* set current entry */
	c_entry = v;
	/* find next entry */
	n_entry = manager_next_entry(c_entry);

	/* return next entry */
	if (n_entry != &manager_init_entry)
		return n_entry;

	/* if next entry == manager_init_entry, terminate */
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
	struct manager_entry *entry = v;
	dbg("");

	/* Display header */
	if (entry == &manager_init_entry) {
		seq_printf(m, "PID \t ProcessName \t "
			"VIRT(KB) \t RSS Mem(KB) \t "
			"DiskRead(KB) \t DiskWrite(KB) \t "
			"Total I/O(KB)\n");
		return 0;
	}

	/* print information */
	seq_printf(m, "%u \t %s \t "
			"%lu \t %lu \t "
			"%llu \t %llu \t "
			"%llu\n",
			entry->pid,
			entry->name,
			entry->virt,
			entry->rss,
			entry->disk_read,
			entry->disk_write,
			entry->total_io);

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