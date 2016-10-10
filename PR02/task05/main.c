#include "sort.h"
#include "pm_list.h"
#include "utils.h"

#include <linux/module.h>
#include <linux/proc_fs.h>

#define PROC_NAME "procmon"

static struct proc_dir_entry *pm;

static int
pm_show(struct seq_file *m, void *v)
{
	dbg("");

	/* show pm_list */
	return pm_list_show(m);
}

static int
pm_open(struct inode *inode, struct file *file)
{
	int ret;
	dbg("");

	/* initialize pm_list
	 * it means to parse all processes & save them in
	 * linked list owned by this module
	*/
	ret = pm_list_init();
	if (ret < 0) {
		err("Failed to initialize pm_list: %d", ret);
		return ret;
	}

	return single_open(file, pm_show, NULL);
}

static int
pm_release(struct inode *inode, struct file *file)
{
	dbg("");

	/* clean-up pm_list */
	pm_list_deinit();

	/* release sequence file */
	return single_release(inode, file);
}

/* file operations */
static struct file_operations fops_pm = {
	.owner = THIS_MODULE,
	.open = pm_open,

	/* read method for sequential files */
	.read = seq_read,

	/* llseek method for sequential files */
	.llseek = seq_lseek,

	/* free the structures associated with sequential file */
	.release = pm_release,
};

static void
_pm_module_exit(void)
{
	dbg("");

	/* de-initialize sorting module */
	sort_module_exit();

	/* remove procmon */
	if (pm != NULL)
		proc_remove(pm);
}

static int __init
pm_module_init(void)
{
	int ret;
	dbg("");

	/* create /proc/procmon */
	pm = proc_create(PROC_NAME, 0, NULL, &fops_pm);
	if (pm == NULL) {
		err("Failed to create procmon");
		goto error;
	}

	/* initialize sorting module */
	ret = sort_module_init();
	if (ret < 0) {
		err("Failed to initialize sorting module");
		goto error;
	}
	return 0;

error:
	_pm_module_exit();
	return -1;
}

static void __exit
pm_module_exit(void)
{
	_pm_module_exit();
}

module_init(pm_module_init);
module_exit(pm_module_exit);

MODULE_AUTHOR("Gaurav Kalra");
MODULE_DESCRIPTION("PR02 Sorting Features");
MODULE_LICENSE("GPL");