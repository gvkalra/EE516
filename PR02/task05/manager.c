#include "manager.h"
#include "utils.h"

#include <linux/sched.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/slab.h>
#include <linux/seq_file.h>

struct manager_entry {
	struct list_head entries;

	pid_t pid;
	char name[TASK_COMM_LEN];
	unsigned long virt;
	long rss;
	unsigned long long disk_read;
	unsigned long long disk_write;
	unsigned long long total_io;
};

static LIST_HEAD(entries_list);

static struct manager_entry *
_create_new_node(pid_t pid, const char *name,
	unsigned long virt, long rss,
	unsigned long long disk_read, unsigned long long disk_write,
	unsigned long long total_io)
{
	struct manager_entry *entry;

	entry = kmalloc(sizeof(struct manager_entry), GFP_KERNEL);
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

void
manager_add_entry(pid_t pid, const char *name,
	unsigned long virt, long rss,
	unsigned long long disk_read, unsigned long long disk_write,
	unsigned long long total_io)
{
	struct manager_entry *entry;

	entry = _create_new_node(pid, name, virt, rss,
		disk_read, disk_write, total_io);
	if (entry == NULL) {
		err("Unable to allocate memory: [%s]", name);
		return;
	}

	list_add(&entry->entries, &entries_list);
}

void
manager_show_monitor(struct seq_file *m, int sort_order)
{
	struct manager_entry *entry = NULL;

	list_for_each_entry(entry, &entries_list, entries) {
		/* print information */
		seq_printf(m, "%s [PID: %u]\n"
			"\tvirt: %lu\n"
			"\trss: %lu\n"
			"\tdisk_read: %llu\n"
			"\tdisk_write: %llu\n"
			"\ttotal_io: %llu\n\n",
			entry->name,
			entry->pid,
			entry->virt,
			entry->rss,
			entry->disk_read,
			entry->disk_write,
			entry->total_io);
	}

	dbg("seq_has_overflowed: [%d]", seq_has_overflowed(m));

#if 0
	struct list_head *i, *j;
	list_for_each_safe(i, j, &entries_list.entries) {
		entry = list_entry(i, struct manager_entry, entries);
		list_del(i);
		kfree(entry);
	}
#endif
}

void manager_init(void)
{
}

void manager_release(void)
{
}