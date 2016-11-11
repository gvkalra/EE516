#include "utils.h"

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

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
#define PHIL_LEFT(i) ((i + PHIL_TOTAL - 1) % PHIL_TOTAL) /* left of philosopher i */
#define PHIL_RIGHT(i) ((i + 1) % PHIL_TOTAL) /* right of philosopher i */

int phil_state[PHIL_TOTAL] = {PHIL_STATE_THINKING,}; /* tracks current state (0,1,2) of philosopher */
sem_t mutex; /* for critical section (initial value = 1) */
sem_t phil_sema[PHIL_TOTAL]; /* for synchronization between philosophers (initial value = 0) */

/* philosopher thread */
static void *philosopher(void *arg);

/* simulation for thinking
 * This is placeholder to mimic thinking process
 * In this case, we sleep() for thinking
 */
static void
simulate_thinking(int id)
{
	info("Philosopher [%d] : Thinking", id);
	sleep((rand() % 2) + 1);
}

/* simulation for eating
 * This is placeholder to mimic eating process
 * In this case, we sleep() for eating
 */
static void
simulate_eating(int id)
{
	info("Philosopher [%d] : Eating", id);
	sleep((rand() % 2) + 1);
}

int main(int argc, const char *argv[])
{
	pthread_t threads[PHIL_TOTAL];
	int i;
	int res;

	/* initialize 'mutex'
	 * initial value 1 means it is a binary semaphore
	 */
	res = sem_init(&mutex, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'phil_sema' */
	for (i = 0; i < PHIL_TOTAL; i++) {
		/* initial value of 0 indicates that
		 * no philosopher is eating */
		res = sem_init(&phil_sema[i], 0, 0);
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

	/* destroy all semaphores */
	sem_destroy(&mutex);
	for (i = 0; i < PHIL_TOTAL; i++) {
		sem_destroy(&phil_sema[i]);
	}

	/* return success */
	return 0;
}

static void
eat_if_possible(int id)
{
	if (phil_state[id] == PHIL_STATE_HUNGRY /* 'id' is hungry */
		&& phil_state[PHIL_LEFT(id)] != PHIL_STATE_EATING /* left is not eating */
		&& phil_state[PHIL_RIGHT(id)] != PHIL_STATE_EATING) /* right is not eating */
	{
		phil_state[id] = PHIL_STATE_EATING;
		/* Notice that PHIL_LEFT(id) has the same value as left side fork
		 * so we can re-use the same macro */
		info("Philosopher [%d] : FORK_UP (%d, %d)", id, PHIL_LEFT(id), id);

		/* signal eating = True for philosopher 'id' */
		sem_post(&phil_sema[id]);
	}
}

static void
take_forks(int id)
{
	sem_wait(&mutex);
	phil_state[id] = PHIL_STATE_HUNGRY;
	eat_if_possible(id);
	sem_post(&mutex);

	/* wait until philosopher 'id' is allowed to eat
	 * who is going to tell if 'id' is allowed to eat now?
	 *     it's neibouring philosophers in put_forks()
	 */
	sem_wait(&phil_sema[id]);
}

static void
put_forks(int id)
{
	sem_wait(&mutex);
	phil_state[id] = PHIL_STATE_THINKING;
	info("Philosopher [%d] : FORK_DOWN (%d, %d)", id, PHIL_LEFT(id), id);

	/* see if neighbouring philosophers can eat
	 * this means -> neighbour should be HUNGRY
	 * and waiting on it's semaphore (phil_sema)
	 */
	eat_if_possible(PHIL_LEFT(id)); /* left can eat? */
	eat_if_possible(PHIL_RIGHT(id)); /* right can eat? */
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
		simulate_thinking(id);

		/* try acquiring forks */
		take_forks(id);

		/* eat */
		simulate_eating(id);

		/* release forks */
		put_forks(id);
	}

	/* unreachable */
	return NULL;
}