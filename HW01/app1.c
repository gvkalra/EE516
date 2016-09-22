#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define STACK_SIZE 256

int main(int argc, char const *argv[])
{
	int item = 0, fd;
	ssize_t bytes_written;

	fd = open("/dev/stack_device", O_WRONLY);
	if (fd < 0) {
        fprintf(stderr, "open() failed err: [%s]\n", strerror(errno));
		return -1;
    }

    /* start pushing numbers to the driver */
    while (item < STACK_SIZE) {
		bytes_written = write(fd, &item, sizeof(item));

		if (bytes_written < 0)
			fprintf(stderr, "write() failed err: [%s]\n", strerror(errno));
		else if (bytes_written != sizeof(item))
			fprintf(stderr, "[NOT WRITTEN] %d\n", item);
		else {
			fprintf(stdout, "[WRITTEN] %d\n", item);
			item++;
		}
    }

	close(fd);
	return 0;
}