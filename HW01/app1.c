#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define STACK_SIZE 256

int main(int argc, const char *argv[])
{
	int item = 0, fd;
	ssize_t bytes_written;

	/* open driver node */
	fd = open("/dev/stack_device", O_WRONLY);
	if (fd < 0) {
        fprintf(stderr, "open() failed err: [%s]\n", strerror(errno));
		return -1;
    }

    /* start pushing numbers to the driver */
    while (item < STACK_SIZE) {
		bytes_written = write(fd, &item, sizeof(item));

		if (bytes_written < 0) /* error */
			fprintf(stderr, "write() failed err: [%s]\n", strerror(errno));
		else if (bytes_written != sizeof(item)) /* partial bytes written */
			fprintf(stderr, "[NOT WRITTEN] %d\n", item);
		else { /* success */
			fprintf(stdout, "[WRITTEN] %d\n", item);
			item++; /* push next item */
		}
    }

    /* cleanup */
	close(fd);
	return 0;
}