#include "utils.h"

#include <pthread.h>
#include <semaphore.h>

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

/*
 * Description:
 *
 * N philosophers are seated around a circular table
 * Each philosophers has a plate of spaghetti
 * Because the spaghetti is too slippery, a philosopher needs two forks to eat it
 * There is one fork between each pair of plates
 * The life of a philosopher consists of alternate period of eating and thinking
 * When a philosopher gets hungry, he tries to acquire his left and right fork, one at a time, in either order
 * If successful in acquiring two forks, he eats for a while, then puts down the forks and continues to think
 *
 * Write a program for each philosopher that does what it is supposed to do and never gets stuck
 * Print a message when a philosopher eats, thinks, and takes left or right fork
 * Test for various number of philosophers (Ex. 4, 5, 6 philosophers)
 */

#define PHIL_TOTAL    5 /* total number of philosophers */

/* states of philosopher */
#define PHIL_STATE_THINKING 0 /* philosopher is thinking */
#define PHIL_STATE_HUNGRY   1 /* philosopher is hungry */
#define PHIL_STATE_EATING   2 /* philosopher is eating */

/* convenience macros */
#define PHIL_RIGHT(i) ((i + PHIL_TOTAL - 1) % PHIL_TOTAL) /* right of philosopher i */
#define PHIL_LEFT(i) ((i + 1) % PHIL_TOTAL) /* left of philosopher i */

int phil_state[PHIL_TOTAL] = {PHIL_STATE_THINKING,}; /* tracks current state (0,1,2) of philosopher */
sem_t mutex; /* for critical section (binary semaphore) */
sem_t fork_sema[PHIL_TOTAL]; /* for ownership of forks */

/* philosopher thread */
static void *philosopher(void *arg);

/* simulation for thinking
 * This is placeholder to mimic thinking process
 * In this case, we sleep() for thinking
 */
static void
__simulate_thinking(int id)
{
	info("Philosopher [%d] : Thinking", id);
	sleep((rand() % 2) + 1);
}

/* simulation for eating
 * This is placeholder to mimic eating process
 * In this case, we sleep() for eating
 */
static void
__simulate_eating(int id)
{
	info("Philosopher [%d] : Eating", id);
	sleep((rand() % 2) + 1);
}

int main(int argc, const char *argv[])
{
	pthread_t threads[PHIL_TOTAL];
	int i;
	int res;

	/* initialize 'mutex' */
	res = sem_init(&mutex, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'fork_sema' */
	for (i = 0; i < PHIL_TOTAL; i++) {
		res = sem_init(&fork_sema[i], 0, 0);
		if (res != 0) {
			perror("sem_init failed.\n");
			exit(1);
		}
	}

	/* initialize rand() seed */
	srand(time(NULL));

	/* run philosopher threads */
	for (i = 0; i < PHIL_TOTAL; i++) {
		pthread_create(&threads[i], NULL, philosopher, (void *)(long)i);
	}

	/* wait for all philosophers to finish
	 * Although we assume spaghetti is unlimited,
	 * in real cases, it may not be unlimited
	*/
	for (i = 0; i < PHIL_TOTAL; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}

static void
__eat_if_you_can(int id)
{
	if (phil_state[id] == PHIL_STATE_HUNGRY /* 'id' is hungry */
		&& phil_state[PHIL_LEFT(id)] != PHIL_STATE_EATING /* left is not eating */
		&& phil_state[PHIL_RIGHT(id)] != PHIL_STATE_EATING) /* right is not eating */
	{
		phil_state[id] = PHIL_STATE_EATING;
		info("Philosopher [%d] : FORK_UP (%d, %d)", id, PHIL_LEFT(id), id);
		sem_post(&fork_sema[id]);
	}
}

static void
__take_forks(int id)
{
	sem_wait(&mutex);
	phil_state[id] = PHIL_STATE_HUNGRY;
	__eat_if_you_can(id);
	sem_post(&mutex);

	sem_wait(&fork_sema[id]);
}

static void
__put_forks(int id)
{
	sem_wait(&mutex);
	phil_state[id] = PHIL_STATE_THINKING;
	info("Philosopher [%d] : FORK_DOWN (%d, %d)", id, PHIL_LEFT(id), id);

	/* be nice to adjacent philosophers */
	__eat_if_you_can(PHIL_LEFT(id)); /* left can eat? */
	__eat_if_you_can(PHIL_RIGHT(id)); /* right can eat? */
	sem_post(&mutex);
}

static void *
philosopher(void *arg)
{
	int id = (int)(long)arg;
	dbg("Running Thread for philosopher: [%d]", id);

	/* forever */
	while (1) {
		/* Think for a while before eating */
		__simulate_thinking(id);

		/* try acquiring forks */
		__take_forks(id);

		/* eat */
		__simulate_eating(id);

		/* release forks */
		__put_forks(id);
	}

	/* unreachable */
	return NULL;
}