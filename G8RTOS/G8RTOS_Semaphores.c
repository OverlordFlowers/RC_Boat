/**
 * G8RTOS_Semaphores.c
 * uP2 - Fall 2022
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <G8RTOS/G8RTOS_CriticalSection.h>
#include <G8RTOS/G8RTOS_Scheduler.h>
#include <G8RTOS/G8RTOS_Semaphores.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"


/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
{
    // your code
    IBit_State = StartCriticalSection();
    (*s) = value;
    EndCriticalSection(IBit_State);
}

/*
 * No longer waits for semaphore
 *  - Decrements semaphore
 *  - Blocks thread if sempahore is unavailable
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_WaitSemaphore(semaphore_t *s)
{

    // your code
    // add your code
    IBit_State = StartCriticalSection();
    (*s)--;
    EndCriticalSection(IBit_State);

    if ((*s) < 0) {
        CurrentlyRunningThread->blocked = s;
        // yield
        HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    }
}

/*
 * Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_SignalSemaphore(semaphore_t *s)
{
    // your code
    IBit_State = StartCriticalSection();
    (*s)++;
    EndCriticalSection(IBit_State);

    tcb_t * CurrentlyConsideredThread = CurrentlyRunningThread->nextTCB;

    while (CurrentlyConsideredThread != CurrentlyRunningThread) {
        if (CurrentlyConsideredThread->blocked == s) {
            // If the blocked semaphore is equal to the signal semaphore address, unblock it
            CurrentlyConsideredThread->blocked = 0;
            // only unblock the first thread blocked on that semaphore.
            break;
        }
        // Change to next tcb
        CurrentlyConsideredThread = CurrentlyConsideredThread->nextTCB;

    }
}

/*********************************************** Public Functions *********************************************************************/


