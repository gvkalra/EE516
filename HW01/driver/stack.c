#include "stack.h"
#include <linux/kernel.h>

#define STACK_SIZE 256

struct {
	int stack[STACK_SIZE];
	int top;
} st = {
	.top = -1,
};

int st_is_empty(void)
{
	printk(KERN_INFO "st_is_empty()\n");

	if (st.top == -1)
		return 1;
	return 0;
}

int st_is_full(void)
{
	printk(KERN_INFO "st_is_full()\n");

	if (st.top >= STACK_SIZE - 1)
		return 1;
	return 0;
}

int st_push(int item)
{
	/* return -1 if stack is already full */
	if (st_is_full())
		return -1;

	printk(KERN_INFO "st_push()\n");

	st.top++;
	st.stack[st.top] = item;
	return 0;
}

int st_pop(int *item)
{
	/* invalid arguments or stack is empty */
	if (item == NULL || st_is_empty())
		return -1;

	printk(KERN_INFO "st_pop()\n");

	*item = st.stack[st.top];
	st.top--;
	return 0;
}

void st_clean(void)
{
	printk(KERN_INFO "st_clean()\n");

	st.top = -1;
}