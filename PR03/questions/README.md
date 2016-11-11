## For "Dining philosopher" problem, how can you prevent starvation?
Starvation may be prevented by giving prefential treatment to the most "starved" philosopher.
And a disadvantage to the philosopher that has just eaten. In other words, philosophers may not be allowed
to eat twice in a row without letting others use the forks in between.

## For "Training monkey" problem, if two male monkeys and two female monkeys can stay in the room, how should your solution be modified?
In my current solution, all monkeys compete for entering the room equally. In other words, all monkeys are competing for 'room'
semaphore, which is initialized to 4 (since 4 monkeys can be present in the room at the same time). If however, we need to
distinguish between male & female monkeys, we can model this by assuming two different semaphores (one for each gender). Male
monkeys outside the room will compete for "male_semaphore" & female monkeys outside the room will compete for "female_semaphore".
In essence, it will be similar to having two different queues (male, female) for entering the training room. Once inside the
room, monkeys compete for same balls irrespective of gender.

## For "Training monkey" problem, if monkeys can enter the room in the ascending order of their IDs, how should your solution be modified?
In my current solution, all monkeys compete for entering the room equally. In other words, all monkeys are competing for 'room'
semaphore, which is initialized to 4 (since 4 monkeys can be present in the room at the same time). If however, monkeys can only
enter the room in the ascending order of their IDs (and we assume all monkeys are ready outside the room), I will not model monkeys
as threads. Since there is a strict order for resource usage (room in this case) already defined, it makes more sense to model
"monkey cages" as threads. In other words, since there are 4 monkeys allowed to be in room at any given time, model these "4 spots"
as threads. These 4 threads will read & write to a common data structure (database of monkeys). Similarly, if we assume a total
order for using 'bowls' as well, we can model bowls as threads which read & write to a shared data structure of monkeys.

## There are POSIX semaphore and non-POSIX semaphore (ex. System V semaphore). What is the difference? Pros and cons?
POSIX is a standard defining APIs, command line shells & utility interfaces for software compatibility with variants of Unix and
other operating systems. As such, POSIX is not the only standard in existence. System V (often known as SysV) is also one
of the standards. Although both POSIX & non-POSIX standard provide almost the same "tools" (e.g. semaphores, shared memory
and message queues), they offer different interfaces to those tools.

As an example, SysV provides the following abstractions (system calls) for semaphore:
semget() - to create a new semaphore set, or access an existing set
semop() - to perform specified operations on selected semaphores
semctl() - to perform control operations on a semaphore set

Whereas, POSIX provides the following abstractions (system calls) for semaphore:
sem_init() - to initialize a semaphore
sem_wait() - to decrement & wait on a semaphore
sem_post() - to increment & wakeup waiting processes on a semaphore

In essence, the difference between POSIX & non-POSIX semaphores is of APIs & implementation in kernel for the same concept.

However, there are a number of subtle pros & cons:
1. In System V you can control how much the semaphore count can be increased or decreased; whereas in POSIX,
the semaphore count is increased and decreased by 1.
2. POSIX semaphores do not allow manipulation of semaphore permissions, whereas System V semaphores allow you to change the
permissions of semaphores to a subset of the original permission.
3. Initialization and creation of semaphores is atomic (from the user's perspective) in POSIX semaphores.
4. From a usage perspective, System V semaphores are clumsy, while POSIX semaphores are straight-forward
5. The scalability of POSIX semaphores (using unnamed semaphores) is much higher than System V semaphores. In a user/client
scenario, where each user creates her own instances of a server, it would be better to use POSIX semaphores.
6. System V semaphores, when creating a semaphore object, creates an array of semaphores whereas POSIX semaphores create just one.
Because of this feature, semaphore creation (memory footprint-wise) is costlier in System V semaphores when compared to POSIX
semaphores.
7. It has been said that POSIX semaphore performance is better than System V-based semaphores.
8. POSIX semaphores provide a mechanism for process-wide semaphores rather than system-wide semaphores. So, if a developer forgets
to close the semaphore, on process exit the semaphore is cleaned up. In simple terms, POSIX semaphores provide a mechanism for
non-persistent semaphores.

References:
http://www.tldp.org/LDP/lpg/node46.html
https://linux.die.net/include/semaphore.h
http://www.linuxdevcenter.com/pub/a/linux/2007/05/24/semaphores-in-linux.html?page=4

## Classify semaphores in task 1 and task 3 into POSIX and non-POSIX semaphore. What is the reason?
Semaphores in task 1 are POSIX semaphores (since they offer APIs as stated in POSIX standard[1]).
On the other hand, task 3 semaphores are _not_ POSIX semaphores. This is because the APIs & implementation is not
according to the POSIX standard.

References:
[1] http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/semaphore.h.html

## An user-level process may access to kernel or hardware directly. What are pros and cons of using system call?
Although it is possible to access hardware directly from user-level process (e.g. by using root account), it is not advisable.
This is because there can be situations, where more than 1 user-level process require access to hardware. In this case, we need to
synchronize between different processes for hardware access. If we implement hardware access in kernel, it is easier to synchronize
between multiple processes. Also, accessing hardware using system call facilitates for easy code reuse.

Cons:
1. You need a syscall number, which needs to be officially assigned to you during a developmental kernel series.
2. System calls are not easily used from scripts and cannot be accessed directly from the filesystem.
3. For simple exchanges of information, a system call is overkill.

For many interfaces, system calls are the correct answer. Linux, however, has tried to avoid simply adding a system call to
support each new abstraction that comes along. The result has been an incredibly clean system call layer with very few regrets or
deprecations (interfaces no longer used or supported). The slow rate of addition of new system calls is a sign that Linux is a
relatively stable and feature-complete operating system.

References:
http://www.makelinux.net/books/lkd2/ch05lev1sec5

## To make a new system call, we compiled entire kernel. What happen if we try to do it with adding a module?
It is not possible because system call table (sys_call_table) is a static size array. And its size is determined at compile
time by the number of registered syscalls. This means there is no space for another one.

There are a few hacks which enable us to add a new system call with a module:
1. Change your kernel to export sys_call_table symbol to modules.
2. Find syscall table dynamically - Iterate over kernel memory, comparing each word with a pointer to known system call function.

However, none of these are recommended in practice and should only be used as a fun way to play with kernel.

References:
http://unix.stackexchange.com/a/48208

## Your own semaphore has several modes. What is pros and cons of each mode? In what situations each mode can take a benefit?
My own semaphore has 3 modes:
1. FIFO
	Pros:
		i) Fairness - 
		ii)
	Cons:
		i)
		ii)
2. User priority
	Pros:
		i)
		ii)
	Cons:
		i)
		ii)
3. OS priority
	Pros:
		i)
		ii)
	Cons:
		i)
		ii)