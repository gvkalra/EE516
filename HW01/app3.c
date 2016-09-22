#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char const *argv[])
{
	int fd, ret;

	fd = open("/dev/stack_device", O_RDONLY);
	if (fd < 0) {
        fprintf(stderr, "open() failed err: [%s]\n", strerror(errno));
		return -1;
    }

    ret = ioctl(fd, 0);
    if (ret < 0) {
    	fprintf(stderr, "ioctl() failed err: [%s]\n", strerror(errno));
    	close(fd);
    	return -1;
    }

	close(fd);
	return 0;
}