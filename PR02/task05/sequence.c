#include "sequence.h"
#include "sorting.h"
#include "utils.h"

#include <linux/task_io_accounting_ops.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/slab.h>

#define INIT_SEQUENCE(e) \
{											\
	.entries = LIST_HEAD_INIT(e.entries),	\
	.pid = 0,								\
	.name = {'\0',},						\
	.virt = 0,								\
	.rss = 0,								\
	.disk_read = 0,							\
	.disk_write = 0,						\
	.total_io = 0,							\
}

struct sequence_entry sequence_init_entry = INIT_SEQUENCE(sequence_init_entry);

static struct sequence_entry *
alloc_sequence_node(pid_t pid, const char *name,
	unsigned long virt, long rss,
	unsigned long long disk_read, unsigned long long disk_write,
	unsigned long long total_io)
{
	struct sequence_entry *entry;

	entry = kmalloc(sizeof(struct sequence_entry), GFP_KERNEL);
	if (entry == NULL)
		return NULL;

	entry->pid = pid;
	memcpy(entry->name, name, TASK_COMM_LEN);
	entry->name[TASK_COMM_LEN - 1] = '\0';
	entry->virt = virt;
	entry->rss = rss;
	entry->disk_read = disk_read;
	entry->disk_write = disk_write;
	entry->total_io = total_io;

	return entry;
}

static void
free_sequence_node(struct sequence_entry *entry)
{
	kfree(entry);
}

static int
sort_sequence_entries(void *priv, struct list_head *a, struct list_head *b)
{
	struct sequence_entry *entry_a, *entry_b;
	int sort_order;

	entry_a = list_entry(a, struct sequence_entry, entries);
	entry_b = list_entry(b, struct sequence_entry, entries);
	sort_order = get_current_sort_order();

	switch (sort_order) {
	case SORT_ORDER_VIRT:
		return (entry_b->virt - entry_a->virt);
	case SORT_ORDER_RSS:
		return (entry_b->rss - entry_a->rss);
	case SORT_ORDER_IO:
		return (entry_b->total_io - entry_a->total_io);
	case SORT_ORDER_PID:
	default:
		return (entry_a->pid - entry_b->pid);
	}
}

int sequence_show(struct seq_file *m, void *v)
{
	struct list_head *cursor, *temp;
	struct sequence_entry *entry;
	dbg("");

	list_for_each_safe(cursor, temp, &sequence_init_entry.entries) {
		entry = list_entry(cursor, struct sequence_entry, entries);

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
	}

	return 0;
}

int sequence_init(void)
{
	struct task_struct *tsk, *t;
	struct sequence_entry *entry;
	char name[TASK_COMM_LEN];
	unsigned long virt;
	long rss;
	struct task_io_accounting acct;

	dbg("");

	/* Add to linked list */
	for_each_process(tsk) {
		dbg("name: [%s] pid: [%d]", get_task_comm(name, tsk),
			task_pid_nr(tsk));

		/* Calculate memory */
		virt = 0;
		rss = 0;
		if (tsk->active_mm != NULL) {
			virt = tsk->active_mm->total_vm;
			rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_FILEPAGES]);
			rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_ANONPAGES]);
		}

		/* Calculate I/O */
		acct = tsk->ioac;
		t = tsk;
		task_io_accounting_add(&acct, &tsk->signal->ioac);
		while_each_thread(tsk, t)
			task_io_accounting_add(&acct, &t->ioac);

		/* Allocate node */
		entry = alloc_sequence_node(task_pid_nr(tsk),
			get_task_comm(name, tsk),
			virt, rss,
			acct.read_bytes, acct.write_bytes,
			(acct.read_bytes + acct.write_bytes));
		if (entry == NULL) {
			info("Skipping PID: [%d]", task_pid_nr(tsk));
			continue;
		}

		/* Add to list */
		list_add(&entry->entries, &sequence_init_entry.entries);
	}

	/* Sort list */
	list_sort(NULL, &sequence_init_entry.entries, sort_sequence_entries);

	return 0;
}

void sequence_deinit(void)
{
	struct list_head *cursor, *temp;
	struct sequence_entry *entry;
	dbg("");

	list_for_each_safe(cursor, temp, &sequence_init_entry.entries) {
		entry = list_entry(cursor, struct sequence_entry, entries);
		dbg("name: [%s] pid: [%d]", entry->name, entry->pid);
		list_del(cursor);
		free_sequence_node(entry);
	}

	dbg("Is empty? %d", list_empty(&sequence_init_entry.entries));
}