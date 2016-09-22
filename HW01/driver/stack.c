#include "stack.h"
#include "utils.h"

#include <linux/module.h>

#define STACK_SIZE 256

struct {
	int stack[STACK_SIZE];
	int top;
} st = {
	.top = -1, /* top is set to -1 to indicate empty stack */
};

int st_is_empty(void)
{
	dbg("");

	if (st.top == -1)
		return 1;
	return 0;
}

int st_is_full(void)
{
	dbg("");

	if (st.top >= STACK_SIZE - 1)
		return 1;
	return 0;
}

int st_push(int item)
{
	dbg("");

	/* add item */
	st.top++;
	st.stack[st.top] = item;
	return 0;
}

int st_pop(int *item)
{
	/* invalid arguments */
	if (item == NULL)
		return -EINVAL;

	dbg("");

	/* remove item */
	*item = st.stack[st.top];
	st.top--;
	return 0;
}

void st_clean(void)
{
	dbg("");

	/* setting top to -1 means stack is cleaned up
	 * there is no way to access st
	*/
	st.top = -1;
}