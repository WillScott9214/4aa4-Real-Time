
#define _GNU_SOURCE
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h> // Used for the MCL conditions
#include <semaphore.h> 

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */
#define MY_PRIORITY1 (42) /* we use max value of 49 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */
#define MY_PRIORITY2 (40)
#define MY_PRIORITY3 (35)
#define NUM_TASKS (3)
const int TASK_PRIORITIES[NUM_TASKS]={MY_PRIORITY1,MY_PRIORITY2,MY_PRIORITY3};

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */
long VALUES[NUM_TASKS] = {50000, 100000, 200000}; 
pthread_mutex_t mutex;
pthread_mutexattr_t mutex_attr;
sem_t semHi_rdy, semMid_rdy, semLow_r, semMid_r, semHi_r; 

void stack_prefault(void) {

        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);
        return;
}

char *PolicyString(int policy){
	switch (policy) {
		case SCHED_OTHER:
			return "SCHED_OTHER";
		//case SCHED_IDLE:
		//	return "SCHED_IDLE";
		//case SCHED_BATCH:
		//			return "SCHED_BATCH";
		case SCHED_FIFO:
			return "SCHED_FIFO";
		case SCHED_RR:
			return "SCHED_RR";
	}
	return "undefined policy";
}

void workfor (long n, int task){ // Use CPU time 
    long counter = 0; 
    while (counter < n){
        counter++; 
        usleep(0); // for making CPU less busy --> This will cause task 2 to start before task 1 is finished
        // to minimize MyRIO connection problems 
    }
    printf("Task %d finished spinning\n", task); 
}

void *task1(void *arg){
    long n = *(long *)arg; 
    struct timespec t;    
    clock_gettime(CLOCK_MONOTONIC ,&t);
    struct timespec old_t = t; 
    printf("Task1 created at: sec: %ld, nsec: %ld\n", (long)t.tv_sec, t.tv_nsec);
    pthread_attr_t getattr;
    struct sched_param param;
    int SchedPolicy;
    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
    }    
    stack_prefault();
    sem_post(&semHi_rdy);
    sem_wait(&semHi_r); 
    pthread_mutex_lock(&mutex); 
    pthread_getattr_np(pthread_self(), &getattr);
    pthread_attr_getschedparam(&getattr,&param);
    pthread_attr_getschedpolicy(&getattr,&SchedPolicy);
    printf("Hello this is Task 1, %s, Priority:%d\n", PolicyString(SchedPolicy), param.sched_priority);
    workfor(n, 1);
    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("Task1 finished at: sec: %ld, nsec: %ld\n", (long)t.tv_sec, t.tv_nsec);
    long t_diff_seconds = (long)t.tv_sec - (long)old_t.tv_sec; 
    long t_diff_nano = t.tv_nsec - old_t.tv_nsec; 
    if (t_diff_nano < 0) { t_diff_seconds--; t_diff_nano += NSEC_PER_SEC; }
    printf("Task1 took %ld seconds, %ld nanoseconds\n", t_diff_seconds, t_diff_nano);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *task2(void *arg){
    long n = *(long *)arg; 
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC ,&t);
    struct timespec old_t = t; 
    printf("Task2 created at: sec: %ld, nsec: %ld\n", (long)t.tv_sec, t.tv_nsec);
    pthread_attr_t getattr;
    struct sched_param param;
    int SchedPolicy;
    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
    }    
    stack_prefault();
    sem_post(&semMid_rdy);
    sem_wait(&semMid_r);
    pthread_getattr_np(pthread_self(), &getattr);
    pthread_attr_getschedparam(&getattr,&param);
    pthread_attr_getschedpolicy(&getattr,&SchedPolicy);
    printf("Hello this is Task 2, %s, Priority:%d\n", PolicyString(SchedPolicy), param.sched_priority);
    workfor(n, 2); 
    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("Task2 finished at: sec: %ld, nsec: %ld\n", (long)t.tv_sec, t.tv_nsec);
    long t_diff_seconds = (long)t.tv_sec - (long)old_t.tv_sec; 
    long t_diff_nano = t.tv_nsec - old_t.tv_nsec; 
    if (t_diff_nano < 0) { t_diff_seconds--; t_diff_nano += NSEC_PER_SEC; }
    printf("Task2 took %ld seconds, %ld nanoseconds\n", t_diff_seconds, t_diff_nano);
    return NULL;
}

void *task3(void *arg){
    long n = *(long *)arg; 
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC ,&t);
    struct timespec old_t = t; 
    printf("Task3 created at: sec: %ld, nsec: %ld\n", (long)t.tv_sec, t.tv_nsec);
    pthread_attr_t getattr;
    struct sched_param param;
    int SchedPolicy;
    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
    }    
    stack_prefault();
    pthread_mutex_lock(&mutex);
    sem_wait(&semLow_r);

    sem_wait(&semHi_rdy);
    sem_wait(&semMid_rdy);

    sem_post(&semHi_r);
    sem_post(&semMid_r);

    pthread_getattr_np(pthread_self(), &getattr);
    pthread_attr_getschedparam(&getattr,&param);
    pthread_attr_getschedpolicy(&getattr,&SchedPolicy);
    printf("Hello this is Task 3, %s, Priority:%d\n", PolicyString(SchedPolicy), param.sched_priority);
    workfor(n, 3); 
    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("Task3 finished at: sec: %ld, nsec: %ld\n", (long)t.tv_sec, t.tv_nsec);
    long t_diff_seconds = (long)t.tv_sec - (long)old_t.tv_sec; 
    long t_diff_nano = t.tv_nsec - old_t.tv_nsec; 
    if (t_diff_nano < 0) { t_diff_seconds--; t_diff_nano += NSEC_PER_SEC; }
    printf("Task3 took %ld seconds, %ld nanoseconds\n", t_diff_seconds, t_diff_nano);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[])
{

    cpu_set_t cpus;
	// Force the program to run on one cpu,
	CPU_ZERO(&cpus); //Initialize cpus to nothing clear previous info if any
	CPU_SET(0, &cpus); // Set cpus to a cpu number zero here
    if (pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT))
    {perror("mutex init"); exit(1);}
    if (pthread_mutex_init(&mutex, &mutex_attr))
    {perror("mutex init"); exit(1);}
	if (sched_setaffinity(0, sizeof(cpu_set_t), &cpus)< 0)
	   perror("set affinity");

    pthread_t pthreadIDS[NUM_TASKS]; 
    void * TASKNAMES[NUM_TASKS] = {task1, task2, task3}; 
    pthread_attr_t attr; 

    sem_init(&semHi_rdy, 0, 0);
    sem_init(&semMid_rdy, 0, 0);
    sem_init(&semLow_r, 0, 0);
    sem_init(&semMid_r, 0, 0);
    sem_init(&semHi_r, 0, 0);

    for (int i=0; i<NUM_TASKS; i++) {
        struct sched_param param; 
        param.sched_priority = TASK_PRIORITIES[i]; 

        pthread_attr_init(&attr);
   		pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
   		pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // Using FIFO
   		pthread_attr_setschedparam(&attr,&param);
   		if (pthread_create(&pthreadIDS[i], &attr, TASKNAMES[i], &VALUES[i])) // This line is where the threads are actually created
   			perror("error in creating pthread ");
   		pthread_attr_destroy(&attr);
    }
    sleep(1);
    sem_post(&semLow_r);

    // Wait for the threads to terminate
   	for (int i=0; i<NUM_TASKS; i++) {
   		pthread_join(pthreadIDS[i], NULL);
   	}
    sem_destroy(&semHi_rdy); 
    sem_destroy(&semMid_rdy);
    sem_destroy(&semLow_r);
    sem_destroy(&semMid_r);
    sem_destroy(&semHi_r);
    return 0; 
}