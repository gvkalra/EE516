/*
	Simple File System Benchmark
	Copyright (C) 1984-2015 Core lab. <djshin.core.kaist.ac.kr>

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>

#define err(fmt,args...) \
	do { \
		fprintf(stderr, "<%s:%d> " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#define info(fmt,args...) \
	do { \
		fprintf(stdout, fmt "\n", ##args); \
	} while (0)

#define NUMFILES 1
#define FILESIZE (1 * 1024 * 1024) /* 1MB */

char *dirname;
int req_size;
ssize_t act_size;

static void
usage(char *prog)
{
	err("Usage: %s <working directory> <request size>",
		prog);
	exit(1);
}

static long
gettimeusec(void)
{
	struct timeval time;
	long usec; 

	gettimeofday(&time, NULL);

	usec = (time.tv_sec * 1000000 + time.tv_usec);
	return usec;
}

/* Generates a unique (non-repeating) random number in O(1)
 *
 * pool_size is how many unique numbers are required to be generated.
 *
 * e.g.
 * The function get_next_rand_number(512) will initialize a pool of 512 random numbers (0 to 511)
 * and return a random number from it.
 * Subsequent calls to get_next_rand_number(-1) will return a unique random number from the pool
 * A call to get_next_rand_number(-1) will return -1 when the pool is exhausted
 *
 * Ref: http://stackoverflow.com/questions/196017/unique-non-repeating-random-numbers-in-o1
*/
static int
get_next_rand_number(int pool_size)
{
	int i, num, temp;
	static int num_left = 0;
	static int *num_pool = NULL;

	/* Initialize a new pool */
	if (pool_size >= 0) {
		/* A pool already exists. Free it first */
		if (num_pool != NULL) {
			free(num_pool);
			num_pool = NULL;
		}

		/* Allocate memory */
		num_pool = malloc(sizeof(int) * pool_size);
		if (num_pool == NULL) {
			err("malloc() failed: [%s]", strerror(errno));
			exit(1);
		}

		/* Initial values */
		for (i = 0; i < pool_size; i++)
			num_pool[i] = i;
		num_left = pool_size;
	}

	/* pool exhausted, return -1 */
	if (num_left == 0)
		return -1;

	/* select a random number between 0 and num_left */
	num = rand() % num_left;

	/* replace num_pool[num] with num_pool[num_left - 1] */
	temp = num_pool[num];
	num_pool[num] = num_pool[num_left - 1];

	/* decrement items left in pool */
	num_left--;

	/* if pool is exhausted, free memory */
	if (num_left == 0) {
		free(num_pool);
		num_pool = NULL;
	}

	/* return previous num_pool[num] (saved in temp) */
	return temp;
}

static void
file_create(void)
{
	int i, fd;
	char filename[128];

	/* Create phase */
	for (i = 0; i < NUMFILES; i++) {
		snprintf(filename, 128, "%s/file-%d", dirname, i);
		fd = open(filename, O_WRONLY | O_CREAT | O_EXCL,
			S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd == -1) {
			err("open() failed: [%s]", strerror(errno));
			exit(1);
		}
		close(fd);
	}
	info("File Created ..");
}

static void
file_delete(void)
{
	int i, ret;
	char filename[128];

	/* Unlink phase */
	for (i = 0; i < NUMFILES; i++) {
		snprintf(filename, 128, "%s/file-%d", dirname, i);
		ret = unlink(filename);
		if (ret == -1) {
			err("unlink() failed: [%s]", strerror(errno));
			exit(1);
		}
	}
}

static void
file_write_sequential(void)
{
	int i, j, fd;
	char filename[128], *buf;
	ssize_t bytes_written;

	buf = memalign((size_t)req_size, (size_t)req_size);
	if (buf == NULL) {
		err("Failed to allocate buffer");
		exit(1);
	}

	for (i = 0; i < NUMFILES; i++) {
		snprintf(filename, 128, "%s/file-%d", dirname, i);
		fd = open(filename, O_DIRECT | O_WRONLY |  O_EXCL,
			S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd == -1) {
			err("open() failed: [%s]", strerror(errno));
			free(buf);
			exit(1);
		}
		info("File Opened Sequential Write ..");

		/* It is possible for write() to return a value > 0 and < req_size,
		 * e.g. if there is insufficient space on the underlying physical medium
		 * refer: http://linux.die.net/man/2/write
		 *
		 * However, no effort has been made to handle that case since this template
		 * is provided as-is in assignment. So, we assume every call to write() will
		 * either flush req_size bytes or fail.
		 * Also, it is assumed that req_size, buf & current file offset are suitably aligned.
		 * e.g. if req_size is passed as 1023, write() will fail with EINVAL
		*/
		for (j = 0; j < FILESIZE; j += req_size) {
			bytes_written = write(fd, buf, req_size);
			if (bytes_written == -1) {
				err("write() failed: [%s]", strerror(errno));
				free(buf);
				close(fd);
				exit(1);
			}
		}

		close(fd);
	}
	free(buf);
}

static void
file_read_sequential(void)
{
	int i, fd;
	char filename[128], *buf;
	ssize_t bytes_read;

	if ((FILESIZE % req_size) != 0) {
		err("FILESIZE(%d) and req_size(%d) are not aligned",
			FILESIZE, req_size);
		exit(1);
	}

	buf = memalign((size_t)req_size, (size_t)req_size);
	if (buf == NULL) {
		err("Failed to allocate buffer");
		exit(1);
	}

	for (i = 0; i < NUMFILES; i++) {
		snprintf(filename, 128, "%s/file-%d", dirname, i);
		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			err("open() failed: [%s]", strerror(errno));
			free(buf);
			exit(1);
		}
		info("File Opened Sequential Read ..");

		do {
			/* read chunks of req_size bytes */
			bytes_read = read(fd, buf, req_size);
			if (bytes_read == -1) {
				err("read() failed: [%s]", strerror(errno));
				free(buf);
				close(fd);
				exit(1);
			}
		/* read until EOF is reached */
		} while (bytes_read != 0);

		close(fd);
	}
	free(buf);
}

static void
file_write_random(void)
{
	int i, offset, fd;
	char filename[128], *buf;
	ssize_t bytes_written;

	if ((FILESIZE % req_size) != 0) {
		err("FILESIZE(%d) and req_size(%d) are not aligned",
			FILESIZE, req_size);
		exit(1);
	}

	buf = memalign((size_t)req_size, (size_t)req_size);
	if (buf == NULL) {
		err("Failed to allocate buffer");
		exit(1);
	}

	for (i = 0; i < NUMFILES; i++) {
		snprintf(filename, 128, "%s/file-%d", dirname, i);
		fd = open(filename, O_DIRECT | O_WRONLY | O_EXCL,
			S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd == -1) {
			err("open() failed: [%s]", strerror(errno));
			free(buf);
			exit(1);
		}
		info("File Opened Random Write ..");

		/* randomly get an offset for lseek() */
		offset = get_next_rand_number((FILESIZE / req_size));
		while (offset >= 0) {
			/* seek from beginning of file */
			lseek(fd, (offset * req_size), SEEK_SET);

			/* at every random offset, flush req_size bytes of data
			*
			* As noted in file_write_sequential(), it is assumed that write()
			* will either flush whole req_size bytes of data or none at all.
			* The case where physical medium is full & there is not enough space
			* is not handled
			*/
			bytes_written = write(fd, buf, req_size);
			if (bytes_written == -1) {
				err("write() failed: [%s]", strerror(errno));
				free(buf);
				close(fd);
				exit(1);
			}

			/* generate another random offset */
			offset = get_next_rand_number(-1);
		}
		close(fd);
	}
	free(buf);
}

static void
file_read_random(void)
{
	int i, fd, offset;
	char filename[128], *buf;
	ssize_t total_bytes, bytes_read;

	if ((FILESIZE % req_size) != 0) {
		err("FILESIZE(%d) and req_size(%d) are not aligned",
			FILESIZE, req_size);
		exit(1);
	}

	buf = memalign((size_t)req_size, (size_t)req_size);
	if (buf == NULL) {
		err("Failed to allocate buffer");
		exit(1);
	}

	for (i = 0; i < NUMFILES; i++) {
		snprintf(filename, 128, "%s/file-%d", dirname, i);
		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			err("open() failed: [%s]", strerror(errno));
			free(buf);
			exit(1);
		}
		info("File Opened Random Read ..");

		/* randomly get an offset for lseek() */
		offset = get_next_rand_number((FILESIZE / req_size));
		while (offset >= 0) {
			/* seek from beginning of file */
			lseek(fd, (offset * req_size), SEEK_SET);

			total_bytes = 0;
			do {
				bytes_read = read(fd, buf, req_size);
				if (bytes_read == -1) {
					err("read() failed: [%s]", strerror(errno));
					free(buf);
					close(fd);
					exit(1);
				}

				/* at every random offset, read req_size bytes of data
				*
				* In case read() is not able to read whole req_size bytes,
				* we retry to read until total_bytes for the current random offset
				* reach req_size. This is best effort basis to sequentially
				* read the whole file.
				*/
				total_bytes += bytes_read;
			} while (total_bytes != req_size);

			/* generate another random offset */
			offset = get_next_rand_number(-1);
		}
		close(fd);
	}
	free(buf);
}

int main(int argc, char **argv)
{
	long creat_time_sequential, write_time_sequential, read_time_sequential;
	long write_time_random, read_time_random, delete_time_random, end_time_random;

	if (argc != 3) {
		usage(argv[0]);
	}
	dirname = argv[1];
	req_size = atoi( argv[2] );

	// Sequential Access Test
	creat_time_sequential = gettimeusec();	
	file_create();	

	write_time_sequential = gettimeusec();	
	file_write_sequential();

	read_time_sequential = gettimeusec();
	file_read_sequential();

	// Random Access Test
	srand(30);	

	write_time_random = gettimeusec();
	file_write_random();

	read_time_random = gettimeusec();
	file_read_random();
	
	delete_time_random = gettimeusec();
	file_delete();

	end_time_random = gettimeusec();

	info("==============  File System Benchmark Execution Result (Time usec)  ==============");
	info("File Create\t : \t%10ld", (write_time_sequential - creat_time_sequential));
	info("Sequential Write : \t%10ld", (read_time_sequential - write_time_sequential));
	info("Sequential Read\t : \t%10ld", (write_time_random - read_time_sequential));
	info("Random Write\t : \t%10ld", (read_time_random - write_time_random));
	info("Random Read\t : \t%10ld", (delete_time_random - read_time_random));
	info("File Delete\t : \t%10ld", (end_time_random - delete_time_random));
	info("Total\t\t : \t%10ld", (end_time_random - creat_time_sequential));
	info("==================================================================================");
}