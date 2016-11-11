#include "utils.h"

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <assert.h>

/*
 * Description:
 *
 * Room logistics:
 *    1) 4 balls (red, green, blue, yellow)
 *    2) 4 monkeys
 * Room training:
 *    1) Each monkey has to take 2 different balls in a given order
 *    2) After a monkey takes one of corresponding balls, it needs to think for a while
 *    3) If a monkey finishes collecting corresponding balls, it will release the balls and leave the room
 *    :: In summary ::
 *           Take the first ball -> Think -> Take the second ball -> Think -> Release balls and leave
 *    4) Whenever a monkey leaves the room, another monkey will enter into the room
 *
 * Outside logistics:
 *    1) 2 feeding bowls
 *    2) unlimited apples (many monkeys can eat apples at the same time)
 * Training process outside room:
 *    1) The monkey leaving the room will face 2 feeding bowls
 *    2) If there is no free bowl, the monkey will eat apples and finish its training
 *    3) If there is a free bowl, the monkey will take it and wait a trainer
 *    4) After the trainer puts bananas in the bowl, the monkey will eat the banana and finish its training
 *    5) The trainer puts bananas if there is a waiting monkey at the empty bowl, or goes to sleep
 * Assume:
 *    1) Since monkeys are well-trained, they will not steal already taken balls (no preemption)
 *    2) Solution should work with N number of monkeys
 */

#define MONKEY_TOTAL 30 /* total number of monkeys */

enum {
	MONKEY_STATE_BORN = 0, /* monkey is just born */
	MONKEY_STATE_READY_ROOM_TRAINING = 1, /* ready to be trained in room */
	MONKEY_STATE_ROOM_TRAINING = 2, /* room training is on-going */
	MONKEY_STATE_THINKING = 3, /* thinking */
	MONKEY_STATE_ROOM_TRAINED = 4, /* room training finished */
};

enum {
	BALL_COLOR_INVALID = -1,
	BALL_COLOR_RED = 0,
	BALL_COLOR_GREEN = 1,
	BALL_COLOR_BLUE = 2,
	BALL_COLOR_YELLOW = 3,
	BALL_COLOR_MAX
};

static const char *color_string[] = {
	[BALL_COLOR_RED] = "RED",
	[BALL_COLOR_GREEN] = "GREEN",
	[BALL_COLOR_BLUE] = "BLUE",
	[BALL_COLOR_YELLOW] = "YELLOW",
};

struct {
	int state; /* idle, thinking, training */
	int color[2]; /* colors of ball interested in */
} monkey_data[MONKEY_TOTAL] = {
	{MONKEY_STATE_BORN, {BALL_COLOR_INVALID, BALL_COLOR_INVALID}},
};

sem_t room; /* for access to room */
sem_t mutex; /* for critical section */
sem_t monkey_sema[MONKEY_TOTAL]; /* for each monkey */

sem_t bowl; /* banana bowl */
int bowl_count = 2;

sem_t trainer; /* trainer */

/* monkey thread */
static void *monkey(void *arg);

/* trainer thread */
static void *trainer(void *arg);

/* simulation for thinking
 * This is placeholder to mimic thinking process
 * In this case, we sleep() for thinking
 */
static void
simulate_monkey_thinking(int id)
{
	info("Monkey %d (%s, %s): thinking", id,
		color_string[monkey_data[id].color[0]],
		color_string[monkey_data[id].color[1]]);
	sleep((rand() % 2) + 1);
}

int main(int argc, const char *argv[])
{
	pthread_t threads[MONKEY_TOTAL];
	int i;
	int res;

	/* initialize 'room'
	 * initial value 4 means 4 monkeys can enter
	 * the room
	 */
	res = sem_init(&room, 0, 4);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'mutex' */
	res = sem_init(&mutex, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'monkey_sema' */
	for (i = 0; i < MONKEY_TOTAL; i++) {
		/* initial value of 0 indicates that
		 * no monkey is training */
		res = sem_init(&monkey_sema[i], 0, 0);
		if (res != 0) {
			perror("sem_init failed.\n");
			exit(1);
		}
	}

	/* initialize 'bowl' */
	res = sem_init(&bowl, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize 'trainer' */
	res = sem_init(&trainer, 0, 1);
	if (res != 0) {
		perror("sem_init failed.\n");
		exit(1);
	}

	/* initialize rand() seed */
	srand(time(NULL));

	/* train monkeys */
	for (i = 0; i < MONKEY_TOTAL; i++) {
		pthread_create(&threads[i], NULL, monkey, (void *)(long)i);
	}

	/* wait for all monkeys to finish */
	for (i = 0; i < MONKEY_TOTAL; i++) {
		pthread_join(threads[i], NULL);
	}

	/* destroy all semaphores */
	sem_destroy(&room);
	sem_destroy(&mutex);
	sem_destroy(&bowl);
	sem_destroy(&trainer);
	for (i = 0; i < MONKEY_TOTAL; i++) {
		sem_destroy(&monkey_sema[i]);
	}

	/* return success */
	return 0;
}

static void
get_two_random_colors(int *color_1, int *color_2)
{
	int color_pool[BALL_COLOR_MAX];
	int i, color, colors_left;

	/* sanity check */
	if (color_1 == NULL || color_2 == NULL) {
		err("Invalid inputs");
		return;
	}

	/* initialize pool */
	for (i = 0; i < BALL_COLOR_MAX; i++)
		color_pool[i] = i;
	colors_left = BALL_COLOR_MAX;

	/* select a random number between 0 and colors_left */
	color = rand() % colors_left;

	/* replace color_pool[color] with color_pool[colors_left - 1] */
	*color_1 = color_pool[color]; /* save color_1 */
	color_pool[color] = color_pool[colors_left - 1];

	colors_left--;

	/* select a random number between 0 and colors_left */
	color = rand() % colors_left;

	/* replace color_pool[color] with color_pool[colors_left - 1] */
	*color_2 = color_pool[color]; /* save color_2 */
	color_pool[color] = color_pool[colors_left - 1];
}

static void
__train_if_you_can(int id)
{
	int color_1, color_2;
	int i, can_train = 1;

	/* this should be called only when monkey is ready to be trained */
	assert(monkey_data[id].state == MONKEY_STATE_READY_ROOM_TRAINING);

	/* find colors monkey is interested in */
	color_1 = monkey_data[id].color[0];
	color_2 = monkey_data[id].color[1];

	/* if no other monkey is training with these colors, 'id' can be trained */
	for (i = 0; i < MONKEY_TOTAL; i++) {
		if (monkey_data[i].state == MONKEY_STATE_ROOM_TRAINING) {
			if (monkey_data[i].color[0] == color_1
				|| monkey_data[i].color[0] == color_2
				|| monkey_data[i].color[1] == color_1
				|| monkey_data[i].color[1] == color_2) {
				can_train = 0;
				break;
			}
		}
	}

	if (can_train) {
		monkey_data[id].state = MONKEY_STATE_ROOM_TRAINING;
		info("Monkey %d (%s, %s): takes the %s ball",
			id, color_string[monkey_data[id].color[0]],
			color_string[monkey_data[id].color[1]],
			color_string[monkey_data[id].color[0]]);
		simulate_monkey_thinking(id);
		info("Monkey %d (%s, %s): takes the %s ball",
			id, color_string[monkey_data[id].color[0]],
			color_string[monkey_data[id].color[1]],
			color_string[monkey_data[id].color[1]]);
		sem_post(&monkey_sema[id]);
	}
}

static void
take_and_think(int id)
{
	int color_1, color_2;

	/* choose 2 random colors */
	get_two_random_colors(&color_1, &color_2);

	/* save data */
	sem_wait(&mutex);
	info("Monkey %d (%s, %s): entered", id,
		color_string[color_1],
		color_string[color_2]);
	monkey_data[id].state = MONKEY_STATE_READY_ROOM_TRAINING;
	monkey_data[id].color[0] = color_1;
	monkey_data[id].color[1] = color_2;
	__train_if_you_can(id);
	sem_post(&mutex);

	/* wait until monkey 'id' is allowed to train
	 * who is going to tell if 'id' is allowed to train now?
	 *     other monkeys inside the room
	*/
	sem_wait(&monkey_sema[id]);
}

static void
release_balls(int id)
{
	int i;

	sem_wait(&mutex);
	monkey_data[id].state = MONKEY_STATE_ROOM_TRAINED;

	info("Monkey %d (%s, %s): balls released", id,
		color_string[monkey_data[id].color[0]],
		color_string[monkey_data[id].color[1]]);

	/* see if other monkeys in room are ready to be trained */
	for (i = 0; i < MONKEY_TOTAL; i++) {
		if (monkey_data[i].state == MONKEY_STATE_READY_ROOM_TRAINING) {
			__train_if_you_can(i);
		}
	}

	sem_post(&mutex);
}

static void *
monkey(void *arg)
{
	int id = (int)(long)arg;
	int received_bowl = 0;

	dbg("Running Thread for monkey: [%d]", id);
	sleep(0); /* give time for other monkeys to queue up */

	/* wait until room is available */
	sem_wait(&room);

	dbg("Monkey [%d] : Entered", id);
	sleep(0); /* give chance for other monkeys to enter */

	/* take balls & think */
	take_and_think(id);

	/* release balls */
	release_balls(id);

	/* leave room */
	info("Monkey %d : left", id);
	sem_post(&room);

	/* see if bowl available */
	sem_wait(&bowl);
	if (bowl_count != 0) {
		bowl_count--;
		received_bowl = 1;
	}
	sem_post(&bowl);

	if (received_bowl) {
		sem_wait(&trainer);
		sem_post(&trainer);
	}

	return NULL;
}

static void *
trainer(void *arg)
{
	
}