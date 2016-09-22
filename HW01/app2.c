#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	int item, fd;
	ssize_t bytes_read;

	fd = open("/dev/stack_device", O_RDONLY);
	if (fd < 0) {
        fprintf(stderr, "open() failed err: [%s]\n", strerror(errno));
		return -1;
    }

    /* start poping numbers from the driver */
    while (1) {
		bytes_read = read(fd, &item, sizeof(item));

		if (bytes_read < 0)
			fprintf(stderr, "read() failed err: [%s]\n", strerror(errno));
		else if (bytes_read == 0) /* EOF */
			break;
		else if (bytes_read != sizeof(item))
			fprintf(stderr, "[NOT READ] %d\n", item);
		else
			fprintf(stdout, "[READ] %d\n", item);
    }

	close(fd);
	return 0;
}