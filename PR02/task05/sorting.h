#pragma once

enum {
	SORT_ORDER_PID = 0,
	SORT_ORDER_VIRT,
	SORT_ORDER_RSS,
	SORT_ORDER_IO
};

inline struct file_operations *
get_sorting_ops(void);

/* Returns SORT_ORDER_<xyz> enumeration */
inline int
get_current_sort_order(void);