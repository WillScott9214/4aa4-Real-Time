/*
 * Copyright (c) 2015,
 * National Instruments Corporation.
 * All rights reserved.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include "DIO.h"
#include "MyRio.h"

#if !defined(LoopDuration)
#define LoopDuration    60  /* How long to output the signal, in seconds */
#endif

#define BLINK_PERIOD_NS (500000000L) // Blink period is > 1ms it's 500ms. 
#define NSEC_PER_SEC (1000000000L)

#define MY_PRIORITY     (49)        /* we use 49 as the PREEMPT_RT use 50
                                       as the priority of kernel tasklets
                                       and interrupt handler by default */

#define MAX_SAFE_STACK  (8*1024)    /* The maximum stack size which is
                                       guaranteed safe to access without
                                       faulting */



typedef struct
{
    MyRio_Dio *channel;     /* DIO channel to toggle */
    int durationSec;        /* How long the task runs, in seconds */
} BlinkTaskArgs;

static void time_increase(struct timespec *t)
{
    t->tv_nsec += BLINK_PERIOD_NS; // increase the clock by 500ms. 
    while (t->tv_nsec >= NSEC_PER_SEC)
    {
        t->tv_nsec -= NSEC_PER_SEC; // Subtract a second from the nanosecond count to avoid overflow
        t->tv_sec++; // add a second onto the clock - just transferring nanoseconds to me
    }

}
void stack_prefault(void)
{
    unsigned char dummy[MAX_SAFE_STACK];

    memset(dummy, 0, MAX_SAFE_STACK);
    return;
}

void *blinkTask(void *arg)
{
    BlinkTaskArgs *args = (BlinkTaskArgs *) arg;

    struct timespec t;

    time_t currentTime;
    time_t finalTime;

    /* Pre-fault this thread's stack */
    stack_prefault();

    clock_gettime(CLOCK_MONOTONIC, &t);

    /* start after one second */
    t.tv_sec++;

    time(&currentTime);
    finalTime = currentTime + args->durationSec;

    while (currentTime < finalTime)
    {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);  
        // void Dio_WriteBit(MyRio_Dio* channel, NiFpga_Bool value)
        Dio_WriteBit(args->channel, 1); // Write to turn on
        time_increase(&t); 

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);  

        Dio_WriteBit(args->channel, 0); // Turn off. 
        time_increase(&t);
        time(&currentTime); 
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t tid; 
    pthread_attr_t attr; 
    
    struct timespec t;
    struct sched_param param;
    int rc; 
    NiFpga_Status status;
    NiFpga_Bool dio_A0;
    NiFpga_Bool dio_A7;
    NiFpga_Bool dio_B5;

    MyRio_Dio A0;
    MyRio_Dio A7;
    MyRio_Dio B5;


    printf("Lab 5 Part 2: Periodic LED blinking w/ registers & threaded (period %ld ms, %d s)\n", 
        BLINK_PERIOD_NS / 1000000L, LoopDuration);
    B5.dir = DIOB_70DIR;
    B5.out = DIOB_70OUT;
    B5.in = DIOB_70IN;
    B5.bit = 5;

    /*
     * Open the myRIO NiFpga Session.
     * This function MUST be called before all other functions. After this call
     * is complete the myRIO target will be ready to be used.
     */
    status = MyRio_Open();
    if (MyRio_IsNotSuccess(status))
    {
        return status;
    }
    BlinkTaskArgs blinkArgs = { &B5, LoopDuration};
    // All this real time stuff is yoinked

    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    {
        perror("mlockall failed");
        MyRio_Close();
        exit(-2);
    }


    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = MY_PRIORITY;
    pthread_attr_setschedparam(&attr, &param);

    /* Create the periodic task as a thread */
    rc = pthread_create(&tid, &attr, blinkTask, &blinkArgs);
    pthread_attr_destroy(&attr);

    if (rc != 0)
    {
        printf("pthread_create failed: %s\n", strerror(rc));
        MyRio_Close();
        exit(-1);
    }

    /* Wait for the periodic task to finish */
    pthread_join(tid, NULL);

    /*
     * Close the myRIO NiFpga Session.
     * This function MUST be called after all other functions.
     */
    status = MyRio_Close();

    /*
     * Returns 0 if successful.
     */
    return status;
}
