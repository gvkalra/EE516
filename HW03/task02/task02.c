#include "utils.h"

#include <malloc.h>
#include <string.h>

/* Make your own program using:
 * [1] malloc()
 * [2] calloc()
 * [3] realloc()
 * [4] free()
 * for allocating and freeing dynamic memory
*/

/* This function prints hex dump of "data"
 * Reference: https://review.tizen.org/gerrit/gitweb?p=platform/core/telephony/libtcore.git
 */
static void
_hex_dump(const char *pad, int size, const void *data)
{
	char buf[255] = {0,};
	char hex[4] = {0,};
	int i = 0;
	unsigned char *p = NULL;

	if (size <= 0) {
		info("%sno data", pad);
		return;
	}

	p = (unsigned char *)data;

	snprintf(buf, 255, "%s%04X: ", pad, 0);
	for (i = 0; i < size; i++) {
		snprintf(hex, 4, "%02X ", p[i]);
		memcpy(buf + strlen(buf), hex, 4);

		if ((i + 1) % 8 == 0) {
			if ((i + 1) % 16 == 0) {
				info("%s", buf);
				memset(buf, 0, 255);
				snprintf(buf, 255, "%s%04X: ", pad, i + 1);
			} else
				strncat(buf, "  ", strlen("  "));
		}
	}
	info("%s", buf);
}

static void
test_malloc()
{
#define REQUESTED_BYTES 128
	void *ptr = NULL;
	info("TESTING : malloc()");

	info("\t Requested : %lu bytes", (long unsigned int)REQUESTED_BYTES);
	ptr = malloc(REQUESTED_BYTES);
	if (ptr != NULL) {
		/* The value returned by malloc_usable_size() may be greater than the
		 * requested size of the allocation because of alignment and minimum
		 * size constraints.  Although the excess bytes can be overwritten by
		 * the application without ill effects, this is not good programming
		 * practice: the number of excess bytes in an allocation depends on the
		 * underlying implementation.
		 * Refer: http://man7.org/linux/man-pages/man3/malloc_usable_size.3.html
		 */
		info("\t Allocated : %lu bytes", malloc_usable_size(ptr));
		_hex_dump("\t ", REQUESTED_BYTES, ptr);

		/* release */
		free(ptr);
	} else {
		err("\t Unable to allocate memory");
	}
#undef REQUESTED_BYTES
}

static void
test_calloc()
{
#define REQUESTED_BYTES 65
#define REQUESTED_BLOCKS 2
	void *ptr = NULL;
	info("TESTING : calloc()");

	info("\t Requested : %lu blocks * %lu bytes = %lu bytes",
		(long unsigned int)REQUESTED_BLOCKS,
		(long unsigned int)REQUESTED_BYTES,
		(long unsigned int)(REQUESTED_BLOCKS * REQUESTED_BYTES));
	ptr = calloc(REQUESTED_BLOCKS, REQUESTED_BYTES);
	if (ptr != NULL) {
		/* The value returned by malloc_usable_size() may be greater than the
		 * requested size of the allocation because of alignment and minimum
		 * size constraints.  Although the excess bytes can be overwritten by
		 * the application without ill effects, this is not good programming
		 * practice: the number of excess bytes in an allocation depends on the
		 * underlying implementation.
		 * Refer: http://man7.org/linux/man-pages/man3/malloc_usable_size.3.html
		 */
		info("\t Allocated : %lu bytes", malloc_usable_size(ptr));
		_hex_dump("\t ", REQUESTED_BLOCKS * REQUESTED_BYTES, ptr);

		/* release */
		free(ptr);
	} else {
		err("\t Unable to allocate memory");
	}
#undef REQUESTED_BLOCKS
#undef REQUESTED_BYTES
}

static void
test_realloc()
{
#define REQUESTED_BYTES 128
#define REVISED_BYTES_1 150
#define REVISED_BYTES_2 128
	void *ptr, *ptr_r = NULL;
	info("TESTING : realloc()");

	info("\t Requested : %lu bytes", (long unsigned int)REQUESTED_BYTES);
	ptr = malloc(REQUESTED_BYTES);
	if (ptr != NULL) {
		/* The value returned by malloc_usable_size() may be greater than the
		 * requested size of the allocation because of alignment and minimum
		 * size constraints.  Although the excess bytes can be overwritten by
		 * the application without ill effects, this is not good programming
		 * practice: the number of excess bytes in an allocation depends on the
		 * underlying implementation.
		 * Refer: http://man7.org/linux/man-pages/man3/malloc_usable_size.3.html
		 */
		info("\t Allocated : %lu bytes", malloc_usable_size(ptr));
		_hex_dump("\t ", REQUESTED_BYTES, ptr);

		/* Reallocate (increase) */
		info("\t Increasing to : %lu bytes", (long unsigned int)REVISED_BYTES_1);
		ptr_r = realloc(ptr, REVISED_BYTES_1);
		if (ptr_r != NULL) {
			info("\t Allocated : %lu bytes", malloc_usable_size(ptr_r));
			_hex_dump("\t ", REVISED_BYTES_1, ptr_r);
		} else {
			err("\t Unable to reallocate memory");
			/* free existing & return */
			free(ptr);
			return;
		}

		/* Reallocate (decrease) */
		info("\t Decreasing to : %lu bytes", (long unsigned int)REVISED_BYTES_2);
		ptr = realloc(ptr_r, REVISED_BYTES_2);
		if (ptr != NULL) {
			info("\t Allocated : %lu bytes", malloc_usable_size(ptr));
			_hex_dump("\t ", REVISED_BYTES_2, ptr);
		} else {
			err("\t Unable to reallocate memory");
			/* free existing & return */
			free(ptr_r);
			return;
		}

		/* release */
		free(ptr);
	} else {
		err("\t Unable to allocate memory");
	}
#undef REVISED_BYTES_2
#undef REVISED_BYTES_1
#undef REQUESTED_BYTES
}

int main(int argc, const char *argv[])
{
	test_malloc();
	test_calloc();
	test_realloc();
	return 0;
}