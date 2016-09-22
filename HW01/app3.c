#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, const char *argv[])
{
	int fd, ret;

	/* open driver node */
	fd = open("/dev/stack_device", O_RDONLY);
	if (fd < 0) {
        fprintf(stderr, "open() failed err: [%s]\n", strerror(errno));
		return -1;
    }

    /* call ioctl() on driver */
    ret = ioctl(fd, 0);
    if (ret < 0) {
    	fprintf(stderr, "ioctl() failed err: [%s]\n", strerror(errno));
    	close(fd);
    	return -1;
    }

    /* cleanup */
	close(fd);
	return 0;
}