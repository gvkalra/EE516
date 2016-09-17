#include "stack.h"
#include <linux/kernel.h>

#define STACK_SIZE 256

struct {
	int stack[STACK_SIZE];
	int top;
} st = {
	.top = -1,
};

int st_push(int item)
{
	st.top++;
	st.stack[st.top] = item;
	return 0;
}

int st_top(int *item)
{
	if (item == NULL)
		return -1;

	*item = st.stack[st.top];
	return 0;
}

int st_pop(int *item)
{
	if (item == NULL)
		return -1;

	*item = st.stack[st.top];
	st.top--;
	return 0;
}

int st_is_empty(void)
{
	if (st.top == -1)
		return 1;
	return 0;
}

int st_is_full(void)
{
	if (st.top >= STACK_SIZE - 1)
		return 1;
	return 0;
}

int st_get_size(void)
{
	//TODO
	return 0;
}