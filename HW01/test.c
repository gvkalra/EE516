#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int item, ret;
	int fd = open("/dev/DUMMY_DEVICE", O_RDWR);

	if (fd <= 0) {
        printf("open() failed");
		return -1;
    }

	item = 254;
//	write(fd, &item, sizeof(item));

	ret = read(fd, &item, sizeof(item));
	printf("ret: [%d] err: [%s] data from the device: %d\n", ret, strerror(errno), item);

	close(fd);
	return 0;
}
