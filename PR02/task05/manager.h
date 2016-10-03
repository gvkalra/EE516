#pragma once

#include <linux/kernel.h>
#include <linux/sched.h>
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

extern struct manager_entry manager_init_entry;

#define manager_next_entry(e) \
	container_of((e)->entries.next, struct manager_entry, entries)

int manager_init(void);
void manager_deinit(void);
int manager_show(struct seq_file *seq, void *v);