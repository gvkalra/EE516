#pragma once

enum {
	SORT_ORDER_PID = 0,
	SORT_ORDER_VIRT,
	SORT_ORDER_RSS,
	SORT_ORDER_IO
};

/* Initializes sorting module
 * Return:
 *     < 0 on error
 */
int
sort_module_init(void);

/* De-initializes sorting module
 */
void
sort_module_exit(void);

/* Returns current sorting order
 * Refer SORT_ORDER_<xyz> enumeration for return values
 */
inline int
sort_get_order(void);