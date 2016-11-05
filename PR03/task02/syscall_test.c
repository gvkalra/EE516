#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define __NR_mycall 322

int main(int argc, const char *argv[])
{
	int n;

	n = syscall(__NR_mycall, 20);
	printf("mycall return value : %d\n", n);

	return 0;
}