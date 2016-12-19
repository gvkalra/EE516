#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>     //@ For system time (random seed)

#define NUMREAD 2			// Number of Readers
#define NUMWRITE 2			// Number of Writers

#define DEVICE_NAME "/dev/DUMMY_DEVICE"
#define NUMLOOP 11
#define RAND_SPAN 10 //Highest random request value is RAND_SPAN-1

/*@ Stores threads' related info @*/
typedef struct thrd_data 
{ 
	int rID [NUMREAD];//@ Array that represents readers' ID numbers
	int wID [NUMWRITE];//@ Array that represents writers' ID numbers
	int fd;//@ Device file descriptor
	
}thrd_data; 

thrd_data thrd;

/* Writer function */
void *write_func(void *arg)
{
	int i, val, ret;
	int ID = *( (int*)arg );//@ Cast *arg to *int and read the pointed int 
	
	for(i=1; i<=NUMLOOP; i++){
		
		val = rand()%RAND_SPAN; 
		ret = write(thrd.fd, &val, sizeof(int)); 
		printf("-->write value: %d, writerID: %d, loop #%d\n",val,ID,i);
		usleep(5000);
	}	
	pthread_exit(NULL);
}

/* Reader function */
void *read_func(void *arg)
{
	int i, val, ret, req;
	int ID = *( (int*)arg );//@ Cast *arg to *int and read the pointed int  
	
	req = rand()%RAND_SPAN;//@ Generate random number [0 ~ RAND_SPAN-1]
	
	for(i=1; i<=NUMLOOP; i++){
		
		ret = read(thrd.fd, &val, req);
		printf(" <--read request: %d, return: %d, readerID: %d, loop #%d\n",req,val,ID,i);
		usleep(5000);
	}	
	pthread_exit(NULL);
}

int main(void)
{
	int i;	
		
	pthread_t read_thread[NUMREAD];
	pthread_t write_thread[NUMWRITE];
	
	srand((unsigned)time(NULL));
	
	/*@ Initilize threads info @*/
	if((thrd.fd = open(DEVICE_NAME,O_RDWR)) < 0){
		printf("open error\n");
		return -1; 
	}
	for(i=0; i<NUMWRITE; i++){
		thrd.wID[i]=i;
	}
	for(i=0; i<NUMREAD; i++){
		thrd.rID[i]=i;
	}
	
	/*@ Create threads @*/
	for(i=0; i<NUMWRITE; i++){
		pthread_create(&write_thread[i], NULL, write_func, (void *)&thrd.wID[i]);
	}
	for(i=0; i<NUMREAD; i++){
		pthread_create(&read_thread[i], NULL, read_func, (void *)&thrd.rID[i]);
	}
	
	/*@ Wait for threads to join @*/
	for(i=0; i<NUMWRITE; i++){
		pthread_join(write_thread[i],NULL);	
	}
	for(i=0; i<NUMREAD; i++){
		pthread_join(read_thread[i],NULL);	
	}
	
	close(thrd.fd);
	return 0;
}


