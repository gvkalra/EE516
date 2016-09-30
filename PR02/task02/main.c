#include "sequence.h"
#include "utils.h"

#include <linux/module.h>
#include <linux/proc_fs.h>

#define PROC_NAME "proc_list"

static struct proc_dir_entry *pl;

static int
pl_open(struct inode *inode, struct file *file)
{
	struct seq_operations *sops;
	dbg("");

	/* initialize sequential file, register operations
	 * Ref: https://www.kernel.org/doc/htmldocs/filesystems/API-seq-open.html
	*/
	sops = get_sequence_ops();
	return seq_open(file, sops);
}

/* file operations */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = pl_open,

	/* read method for sequential files */
	.read = seq_read,

	/* llseek method for sequential files */
	.llseek = seq_lseek,

	/* free the structures associated with sequential file */
	.release = seq_release,
};

static void
_pl_module_exit(void)
{
	dbg("");

	if (pl != NULL)
		proc_remove(pl);
}

static int __init
pl_module_init(void)
{
	dbg("");

	pl = proc_create(PROC_NAME, 0, NULL, &fops);
	if (pl == NULL) {
		err("Failed to create proc_list");
		goto error;
	}
	return 0;

error:
	_pl_module_exit();
	return -1;
}

static void __exit
pl_module_exit(void)
{
	_pl_module_exit();
}

module_init(pl_module_init);
module_exit(pl_module_exit);

MODULE_AUTHOR("Gaurav Kalra");
MODULE_DESCRIPTION("PR02 Traverse Process - tasklist");
MODULE_LICENSE("GPL");