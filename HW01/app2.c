#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	int item, fd;
	ssize_t bytes_read;

	/* open driver node */
	fd = open("/dev/stack_device", O_RDONLY);
	if (fd < 0) {
        fprintf(stderr, "open() failed err: [%s]\n", strerror(errno));
		return -1;
    }

    /* start poping numbers from the driver */
    while (1) {
		bytes_read = read(fd, &item, sizeof(item));

		if (bytes_read < 0) /* error */
			fprintf(stderr, "read() failed err: [%s]\n", strerror(errno));
		else if (bytes_read == 0) /* EOF */
			break;
		else if (bytes_read != sizeof(item)) /* incomplete data */
			fprintf(stderr, "[NOT READ] %d\n", item);
		else /* success */
			fprintf(stdout, "[READ] %d\n", item);
    }

    /* cleanup */
	close(fd);
	return 0;
}