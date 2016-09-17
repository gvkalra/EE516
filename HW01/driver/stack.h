#pragma once

/* Adds an item onto the stack
 * Returns < 0 on error
*/
int st_push(int item);

/* Returns the last item pushed onto the stack (as out argument)
 * Returns < 0 on error
*/
int st_top(int *item);

/* Removes the most-recently-pushed item from the stack (as out argument)
 * Returns < 0 on error
*/
int st_pop(int *item);

/* True if no more items can be popped and there is no top item
*/
int st_is_empty(void);

/* True if no more items can be pushed
*/
int st_is_full(void);

/* Returns the number of elements on the stack
*/
int st_get_size(void);