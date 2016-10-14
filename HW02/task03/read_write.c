#include "utils.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

/*
 * Assume that:
 *     1. There are 3 readers and 3 writers
 *     2. The size of database is only 1 byte
 *     3. Writer-i writes i to the database every 1 second
 *     4. Readers read database randomly
*/

/* reader thread */
static void *reader(void *arg);

/* writer thread */
static void *writer(void *arg);

sem_t mutex; /* initial value = 1; access control to 'rc' */
sem_t db; /* initial value = 1; access control to database */
int rc = 0; /* # of processes reading or wanting to */

uint8_t buffer; /* 1 byte */

int main(int argc, const char *argv[])
{
	int i;
	pthread_t threads[6];
	int res;

	/* initialize 'mutex' */
	res = sem_init(&mutex, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'db' */
	res = sem_init(&db, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* readers */
	pthread_create(&threads[0], NULL, reader, NULL);
	pthread_create(&threads[1], NULL, reader, NULL);
	pthread_create(&threads[2], NULL, reader, NULL);

	/* writers */
	pthread_create(&threads[3], NULL, writer, NULL);
	pthread_create(&threads[4], NULL, writer, NULL);
	pthread_create(&threads[5], NULL, writer, NULL);

	/* wait for threads to exit */
	for (i = 0; i < 6; i++)
		pthread_join(threads[i], NULL);

	/* destroy all semaphores */
	sem_destroy(&db);
	sem_destroy(&mutex);

	/* return success */
	return 0;
}

static void *
reader(void *arg)
{
	dbg("");
	return NULL;
}

static void *
writer(void *arg)
{
	dbg("");
	return NULL;
}