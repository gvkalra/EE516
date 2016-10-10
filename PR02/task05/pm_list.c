#include "pm_list.h"
#include "sort.h"
#include "utils.h"

#include <linux/task_io_accounting_ops.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/slab.h>

/* initilizes pm_list_init_entry */
#define INIT_PM_LIST(e) \
{											\
	.entries = LIST_HEAD_INIT(e.entries),	\
	.pid = 0,								\
	.name = {'\0',},						\
	.virt = 0,								\
	.rss = 0,								\
	.disk_read = 0,							\
	.disk_write = 0,						\
}
struct pm_list_entry pm_list_init_entry = INIT_PM_LIST(pm_list_init_entry);

/* sort function for pm_list */
static int
sort_pm_list_entries(void *priv, struct list_head *a, struct list_head *b)
{
	struct pm_list_entry *entry_a, *entry_b;
	int sort_order;

	/* typecast */
	entry_a = list_entry(a, struct pm_list_entry, entries);
	entry_b = list_entry(b, struct pm_list_entry, entries);

	/* find current sort order */
	sort_order = sort_get_order();

	/* sort */
	switch (sort_order) {
	case SORT_ORDER_VIRT:
		return (entry_b->virt - entry_a->virt);

	case SORT_ORDER_RSS:
		return (entry_b->rss - entry_a->rss);

	case SORT_ORDER_IO:
		/* subtract & then add to prevent overflow */
		return ((entry_b->disk_write - entry_a->disk_write) + \
			(entry_b->disk_read - entry_a->disk_read));

	case SORT_ORDER_PID:
	default:
		return (entry_a->pid - entry_b->pid);
	}
}

int
pm_list_show(struct seq_file *m)
{
	struct list_head *cursor, *temp;
	struct pm_list_entry *entry;
	dbg("");

	/* Sort list */
	list_sort(NULL,
		&pm_list_init_entry.entries,
		sort_pm_list_entries);

	/* title */
	seq_printf(m, "================ Process Monitoring Manager for EE516 ================\n");

	/* header */
	seq_printf(m, "PID       ProcessName         VIRT(KB)            "
		"RSS Mem(KB)         DiskRead(KB)        DiskWrite(KB)       "
		"Total I/O(KB)       \n");

	list_for_each_safe(cursor, temp, &pm_list_init_entry.entries) {
		/* typecast */
		entry = list_entry(cursor, struct pm_list_entry, entries);

		/* print information */
		seq_printf(m, "%-10u%-20s%-20llu%-20llu%-20llu%-20llu%-20llu\n",
		entry->pid,
		entry->name,
		entry->virt,
		entry->rss,
		entry->disk_read,
		entry->disk_write,
		(entry->disk_read + entry->disk_write));
	}

	return 0;
}

static struct pm_list_entry *
alloc_pm_list_node(pid_t pid, const char *name,
	unsigned long long virt, long long rss,
	unsigned long long disk_read, unsigned long long disk_write)
{
	struct pm_list_entry *entry;

	/* allocate memory */
	entry = kmalloc(sizeof(struct pm_list_entry), GFP_KERNEL);
	if (entry == NULL)
		return NULL;

	/* assign values */
	entry->pid = pid;
	memcpy(entry->name, name, TASK_COMM_LEN);
	entry->name[TASK_COMM_LEN - 1] = '\0';
	entry->virt = virt;
	entry->rss = rss;
	entry->disk_read = disk_read;
	entry->disk_write = disk_write;

	/* return node */
	return entry;
}

static void
free_pm_list_node(struct pm_list_entry *entry)
{
	kfree(entry);
}

int
pm_list_init(void)
{
	struct task_struct *tsk, *t;
	struct pm_list_entry *entry;
	char name[TASK_COMM_LEN];
	unsigned long long virt;
	long long rss;
	struct task_io_accounting acct;

	dbg("");

	/* Add to linked list */
	for_each_process(tsk) {
		dbg("[ADD] name: [%s] pid: [%d]", get_task_comm(name, tsk),
			task_pid_nr(tsk));

		/* Calculate memory */
		virt = 0;
		rss = 0;
		if (tsk->active_mm != NULL) {
			virt = tsk->active_mm->total_vm;
			virt *= (PAGE_SIZE >> 10); /* convert pages to KB */

			rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_FILEPAGES]);
			rss += atomic_long_read(&tsk->active_mm->rss_stat.count[MM_ANONPAGES]);
			rss *= (PAGE_SIZE >> 10); /* convert pages to KB */
		}

		/* I/O */
		acct = tsk->ioac;
		t = tsk;

		/* Account for each thread */
		task_io_accounting_add(&acct, &tsk->signal->ioac);
		while_each_thread(tsk, t)
			task_io_accounting_add(&acct, &t->ioac);

		/* Allocate linked list node */
		entry = alloc_pm_list_node(task_pid_nr(tsk),
					get_task_comm(name, tsk),
					virt, rss,
					(acct.read_bytes >> 10), /* divide by 1024 to convert into KB */
					(acct.write_bytes >> 10));
		if (entry == NULL) {
			info("Skipping PID: [%d]", task_pid_nr(tsk));
			continue;
		}

		/* Add to list */
		list_add(&entry->entries, &pm_list_init_entry.entries);
	}

	return 0;
}

void
pm_list_deinit(void)
{
	struct list_head *cursor, *temp;
	struct pm_list_entry *entry;
	dbg("");

	list_for_each_safe(cursor, temp, &pm_list_init_entry.entries) {
		entry = list_entry(cursor, struct pm_list_entry, entries);
		dbg("[REMOVE] name: [%s] pid: [%d]", entry->name, entry->pid);

		list_del(cursor);
		free_pm_list_node(entry);
	}

	dbg("Is empty? %d", list_empty(&pm_list_init_entry.entries));
}