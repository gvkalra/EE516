#pragma once

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/seq_file.h>

struct sequence_entry {
	struct list_head entries;

	pid_t pid;
	char name[TASK_COMM_LEN];
	unsigned long virt;
	long rss;
	unsigned long long disk_read;
	unsigned long long disk_write;
	unsigned long long total_io;
};

extern struct sequence_entry sequence_init_entry;

#define sequence_next_entry(e) \
	container_of((e)->entries.next, struct sequence_entry, entries)

int sequence_init(void);
void sequence_deinit(void);
int sequence_show(struct seq_file *seq, void *v);