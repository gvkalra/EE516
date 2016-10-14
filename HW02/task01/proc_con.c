#include "utils.h"

#include <pthread.h>
#include <semaphore.h>

/*
 * Assume that there are one producer and one consumer,
 * where each producer and consumer produces and consumes 100 items respectively.
*/

void *producer(void *arg);
void *consumer(void *arg);

sem_t mutex;
sem_t empty;
sem_t full;

int buffer[100];
int count = 0;

int main(int argc, const char *argv[])
{
	int i;
	pthread_t threads[2];
	int res;

	res = sem_init(&mutex, 0, 1);
	if (res !=0 ) {
		perror("sem_init failed.\n");
		exit(1);
	}

	res = sem_init(&empty, 0, 100);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	res = sem_init(&full, 0, 0);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	pthread_create(&threads[0], NULL, producer, NULL);
	pthread_create(&threads[1], NULL, consumer, NULL);

	for (i = 0; i < 2; i++)
		pthread_join(threads[i], NULL);

	sem_destroy(&mutex);
	sem_destroy(&empty);
	sem_destroy(&full);

	return 0;
}

void *producer(void *arg)
{
	int i;
	dbg("");

	for (i = 0; i < 300; i++) {
	}

	return NULL;
}

void *consumer(void *arg)
{
	int i;
	dbg("");

	for (i = 0; i < 300; i++) {
	}

	return NULL;
}