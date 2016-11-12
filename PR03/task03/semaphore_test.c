#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#include <pthread.h>
#include <assert.h>

#define __NR_mysema_init 322
#define __NR_mysema_down 323
#define __NR_mysema_down_userprio 324
#define __NR_mysema_up 325
#define __NR_mysema_release 326

#define MYSEMA_MODE_FIFO 0
#define MYSEMA_MODE_OS 1
#define MYSEMA_MODE_USR 2

static inline int
mysema_init(int sema_id, int start_value, int mode)
{
	return syscall(__NR_mysema_init, sema_id, start_value, mode);
}

static inline int
mysema_down(int sema_id)
{
	return syscall(__NR_mysema_down, sema_id);
}

static inline int
mysema_down_userprio(int sema_id, int priority)
{
	return syscall(__NR_mysema_down_userprio, sema_id, priority);
}

static inline int
mysema_up(int sema_id)
{
	return syscall(__NR_mysema_up, sema_id);
}

static inline int
mysema_release(int sema_id)
{
	return syscall(__NR_mysema_release, sema_id);
}

static void
test_init_release()
{
	int ret[11], i;

	/* Test initialization, release */
	for (i = 0; i < 11; i++) {
		ret[i] = mysema_init(i, 0, MYSEMA_MODE_FIFO);
		if (i <= 9) {
			assert(ret[i] == 0);
		} else {
			assert(ret[i] == -1); /* we permit from 0-to-9 only */
		}

		ret[i] = mysema_init(i, 0, MYSEMA_MODE_FIFO);
		assert(ret[i] == -1); /* reinitialization is not permitted */

		ret[i] = mysema_release(i);
		if (i <= 9) {
			assert(ret[i] == 0);
		} else {
			assert(ret[i] == -1); /* we permit from 0-to-9 only */
		}

		ret[i] = mysema_release(i);
		assert(ret[i] == -1); /* re-release is not permitted */
	}

	printf("test_init_release() : PASSED\n");
}

static void *
test_fifo_thread(void *arg)
{
	int ret = mysema_up(0);
	assert(ret == 0);
	return NULL;
}

static void
test_fifo()
{
	pthread_t threads[2];
	int ret;

	ret = mysema_init(0, 0, MYSEMA_MODE_FIFO);
	assert(ret == 0);

	pthread_create(&threads[0], NULL, test_fifo_thread, (void *)(long)0);
	pthread_create(&threads[1], NULL, test_fifo_thread, (void *)(long)1);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	ret = mysema_release(0);
	assert(ret == 0);

	printf("test_fifo() : PASSED\n");
}

static void *
test_user_prio_thread(void *arg)
{
	int ret = mysema_down_userprio(0, 100);
	assert(ret == 0);

	ret = mysema_up(0);
	assert(ret == 0);
	return NULL;
}

static void
test_user_prio()
{
	pthread_t thread;
	int ret;

	ret = mysema_init(0, 1, MYSEMA_MODE_USR);
	assert(ret == 0);

	pthread_create(&thread, NULL, test_user_prio_thread, NULL);
	pthread_join(thread, NULL);

	ret = mysema_release(0);
	assert(ret == 0);

	printf("test_user_prio() : PASSED\n");
}

static void *
test_os_prio_thread(void *arg)
{
	int ret = mysema_down(0);
	assert(ret == 0);

	ret = mysema_up(0);
	assert(ret == 0);
	return NULL;
}

static void
test_os_prio()
{
	pthread_t thread;
	int ret;

	ret = mysema_init(0, 1, MYSEMA_MODE_OS);
	assert(ret == 0);

	pthread_create(&thread, NULL, test_os_prio_thread, NULL);
	pthread_join(thread, NULL);

	ret = mysema_release(0);
	assert(ret == 0);

	printf("test_os_prio() : PASSED\n");
}

int main(int argc, const char *argv[])
{
	test_init_release();
	test_fifo();
	test_user_prio();
	test_os_prio();

	return 0;
}