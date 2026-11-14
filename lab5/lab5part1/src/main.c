/*
 * Copyright (c) 2015,
 * National Instruments Corporation.
 * All rights reserved.
 */

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


int main(int argc, char **argv)
{

    struct timespec t;
    struct sched_param param;
    NiFpga_Status status;
    NiFpga_Bool dio_A0;
    NiFpga_Bool dio_A7;
    NiFpga_Bool dio_B7;

    MyRio_Dio A0;
    MyRio_Dio A7;
    MyRio_Dio B7;

    time_t currentTime;
    time_t finalTime;

    printf("Lab 5 Part 1: Periodic LED blinking w/ registers (period %ld ms, %d s)\n", 
        BLINK_PERIOD_NS / 1000000L, LoopDuration);
    B7.dir = DIOB_70DIR;
    B7.out = DIOB_70OUT;
    B7.in = DIOB_70IN;
    B7.bit = 7;

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
    // All this real time stuff is yoinked
    /* Declare ourself as a real time task */
    param.sched_priority = MY_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
    {
        perror("sched_setscheduler failed");
        MyRio_Close();
        exit(-1);
    }

    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    {
        perror("mlockall failed");
        MyRio_Close();
        exit(-2);
    }

    /* Pre-fault our stack */
    stack_prefault();

    clock_gettime(CLOCK_MONOTONIC, &t); 
    t.tv_sec++; 

    time(&currentTime);
    finalTime = currentTime + LoopDuration;
    while (currentTime < finalTime)
    {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL); // t is like an absolute time! So we do t = t+1 before the loop, then here it is like a 1s duration. 
        // void Dio_WriteBit(MyRio_Dio* channel, NiFpga_Bool value)
        Dio_WriteBit(&B7, 1); // Write to turn on
        time_increase(&t); 

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);  

        Dio_WriteBit(&B7, 0); // Turn off. 
        time_increase(&t);
        time(&currentTime); 
    }

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
