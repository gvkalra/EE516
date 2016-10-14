#include "utils.h"

#include <pthread.h>
#include <semaphore.h>

/*
 * Assume that there is one producer and one consumer,
 * where each producer produces 100 items
 * and each consumer consumes 100 items
*/

/* producer thread */
static void *producer(void *arg);

/* consumer thread */
static void *consumer(void *arg);

sem_t mutex; /* initial value = 1 */
sem_t empty; /* initial value = 100 */
sem_t full; /* initial value = 0 */

int buffer[100];
int count = 0; /* number of items in the buffer */

int main(int argc, const char *argv[])
{
	int i;
	pthread_t threads[2];
	int res;

	/* initialize 'mutex' */
	res = sem_init(&mutex, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'empty' */
	res = sem_init(&empty, 0, 100);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'full' */
	res = sem_init(&full, 0, 0);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	pthread_create(&threads[0], NULL, producer, NULL);
	pthread_create(&threads[1], NULL, consumer, NULL);

	/* wait for threads to exit */
	for (i = 0; i < 2; i++)
		pthread_join(threads[i], NULL);

	/* destroy all semaphores */
	sem_destroy(&mutex);
	sem_destroy(&empty);
	sem_destroy(&full);

	/* return success */
	return 0;
}

static void *
producer(void *arg)
{
	int i;
	dbg("");

	for (i = 0; i < 100; i++) {
		sem_wait(&empty);
		sem_wait(&mutex);

		/* insert */
		buffer[count] = i;
		count++;
		dbg("[P] : %d", i);

		sem_post(&mutex);
		sem_post(&full);
	}

	return NULL;
}

static void *
consumer(void *arg)
{
	int i, item;
	dbg("");

	for (i = 0; i < 100; i++) {
		sem_wait(&full);
		sem_wait(&mutex);

		/* remove */
		count--;
		item = buffer[count];
		dbg("[C] : %d", item);

		sem_post(&mutex);
		sem_post(&empty);
	}

	return NULL;
}