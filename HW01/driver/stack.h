#pragma once

/* True if no more items can be popped and there is no top item
*/
int st_is_empty(void);

/* True if no more items can be pushed
*/
int st_is_full(void);

/* Adds an item onto the stack
 * Returns < 0 on error
*/
int st_push(int item);

/* Removes the most-recently-pushed item from the stack (as out argument)
 * Returns < 0 on error
*/
int st_pop(int *item);

/* Cleans up stack
*/
void st_clean(void);