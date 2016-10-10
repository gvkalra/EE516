#pragma once

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/seq_file.h>

/* pm_list node entry */
struct pm_list_entry {
	struct list_head entries;

	pid_t pid;
	char name[TASK_COMM_LEN];
	unsigned long long virt; /* in KB */
	long long rss; /* in KB */
	unsigned long long disk_read; /* in KB */
	unsigned long long disk_write; /* in KB */
};

/* first entry of pm_list */
extern struct pm_list_entry pm_list_init_entry;

/* macro to find next pm_list element */
#define pm_list_next_entry(e) \
	container_of((e)->entries.next, struct pm_list_entry, entries)

/* initializes pm_list
 * Return:
 *    < 0 on error
*/
int pm_list_init(void);

/* deinits pm_list
*/
void pm_list_deinit(void);

/* shows pm_list after sorting in
 * currently set order
*/
int pm_list_show(struct seq_file *m);