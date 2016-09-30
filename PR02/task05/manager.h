#pragma once

#include <linux/kernel.h>

void
manager_add_entry(pid_t pid, const char *name,
	unsigned long virt, long rss,
	unsigned long long disk_read, unsigned long long disk_write,
	unsigned long long total_io);

void
manager_show_monitor(int sort_order);

void manager_init(void);
void manager_release(void);