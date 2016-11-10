#include "utils.h"

#include <pthread.h>
#include <semaphore.h>

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

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
 *    1) Since monkeys are well-trained, they will not steal already taken balls
 *    2) Solution should work with N number of monkeys
 */

int main(int argc, const char *argv[])
{
	return 0;
}