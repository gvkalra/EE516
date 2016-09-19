#pragma once

#include <linux/kernel.h>

#define DEBUG_ENABLE /* comment this to disable debugging logs */

#ifdef DEBUG_ENABLE
	#define dbg(fmt,args...) \
		do { \
			printk(KERN_DEBUG "<%s:%d> " fmt "\n", __func__, __LINE__, ##args); \
		} while (0)
#else
	#define dbg(fmt,args...)
#endif

#define err(fmt,args...) \
	do { \
		printk(KERN_ERR "<%s:%d> " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#define info(fmt,args...) \
	do { \
		printk(KERN_INFO "<%s:%d> " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)
