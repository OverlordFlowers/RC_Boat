/**
 * G8RTOS_Scheduler.c
 * uP2 - Fall 2022
 */
#include <G8RTOS/G8RTOS_CriticalSection.h>
#include <G8RTOS/G8RTOS_Scheduler.h>
#include <G8RTOS/G8RTOS_Structures.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/*
 * G8RTOS_Start exists in asm
 */
extern void G8RTOS_Start();

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/*
 * Pointer to the currently running Thread Control Block
 */
extern tcb_t * CurrentlyRunningThread;

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];

/* Thread Stacks
 *	- An array of arrays that will act as individual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Periodic Event Threads
 * - An array of periodic events to hold pertinent information for each thread
 */
static ptcb_t Pthread[MAXPTHREADS];

//static uint32_t* AperiodicThreads[MAX_USERIRQ];

/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;

/*
 * Current Number of Periodic Threads currently in the scheduler
 */
static uint32_t NumberOfPthreads;

/*
 * Current ID num
 */
static threadId_t threadID = 0;

/*********************************************** Private Variables ********************************************************************/


/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles)
{
    // your code
    uint32_t clock_freq = SysCtlClockGet();
    // Period = S_clk / Prescaler * Desired time
    //uint32_t period = (clock_freq / 1) / 1000;

    SysTickPeriodSet(numCycles);
    SysTickIntRegister(SysTick_Handler);
    IntRegister(FAULT_PENDSV, PendSV_Handler);
    SysTickIntEnable();
    SysTickEnable();

}

/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 * 	- Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 * 	- Check for sleeping and blocked threads
 */
void G8RTOS_Scheduler()
{
    uint16_t nextThreadPriority = UINT8_MAX + 1;
    tcb_t* nextThread = CurrentlyRunningThread->nextTCB;

    for (int i = 0; i < NumberOfThreads; i++) {
        if (nextThread->blocked == 0 && nextThread->asleep == 0) {
            if (nextThread->priority < nextThreadPriority) {
                CurrentlyRunningThread = nextThread;
                nextThreadPriority = CurrentlyRunningThread->priority;
            }
        }
        nextThread = nextThread->nextTCB;
    }
}


/*
 * SysTick Handler
 * The Systick Handler now will increment the system time,
 * set the PendSV flag to start the scheduler,
 * and be responsible for handling sleeping and periodic threads
 */
void SysTick_Handler()
{
    SystemTime++;

    tcb_t* currThread = CurrentlyRunningThread;

    for (int i = 0; i < NumberOfThreads; i++) {
        if (currThread->sleepCount <= SystemTime) {
            currThread->asleep = 0;
        }
        currThread = currThread->nextTCB;
    }

    for (int i = 0; i < NumberOfPthreads; i++) {
        if (Pthread[i].executeTime <= SystemTime) {
            Pthread[i].handler();
            Pthread[i].currentTime = SystemTime;
            Pthread[i].executeTime = SystemTime + Pthread[i].period;
        }
    }
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

/*********************************************** Private Functions ********************************************************************/


/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;

/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/


/*
 * SysTick Handler
 * The Systick Handler now will increment the system time,
 * Set the PendSV flag to start the scheduler,
 * And be responsible for handling sleeping and periodic threads
 */


/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{

    uint32_t newVTORTable = 0x20000000;
    uint32_t* newTable = (uint32_t*)newVTORTable;
    uint32_t* oldTable = (uint32_t*) 0;

    for (int i = 0; i < 155; i++) {
        newTable[i] = oldTable[i];
    }
    HWREG(NVIC_VTABLE) = newVTORTable;


    SystemTime = 0;
    NumberOfThreads = 0;
    NumberOfPthreads = 0;
}


/*
 * Starts G8RTOS Scheduler
 *  - Initializes the Systick
 *  - Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int G8RTOS_Launch()
{
    InitSysTick(SysCtlClockGet() / 1000); // 1 ms tick (1Hz / 1000)

    // add your code
    CurrentlyRunningThread = &threadControlBlocks[0];
    IntPrioritySet(FAULT_SYSTICK, OSINT_PRIORITY);
    IntPrioritySet(FAULT_PENDSV, OSINT_PRIORITY);
    G8RTOS_Start(); // call the assembly function
    return 0;
}



/* G8RTOS_AddThread
 *  - Adds threads to G8RTOS Scheduler
 *  - Checks if there are still available threads to insert to scheduler
 *  - Initializes the thread control block for the provided thread
 *  - Initializes the stack for the provided thread to hold a "fake context"
 *  - Sets stack thread control block stack pointer to top of thread stack
 *  - Sets up the next and previous thread control block pointers in a round robin fashion
 * Parameter "threadToAdd": Void-Void Function to add as pre-emptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name)
{
    IBit_State = StartCriticalSection();
    uint8_t threadIndex = 0;

    if (GetNumberOfThreads() >= MAX_THREADS) {
            EndCriticalSection(IBit_State);
            return THREAD_LIMIT_REACHED;
    } else {
        if (GetNumberOfThreads() == 0) {
            threadControlBlocks[0].isAlive = 1;
            threadControlBlocks[0].nextTCB = &(threadControlBlocks[0]);
            threadControlBlocks[0].previousTCB = &(threadControlBlocks[0]);
        } else {
            for (int i = 1; i < MAX_THREADS; i++) {
                if (!threadControlBlocks[i].isAlive) {
                    // previous thread is just i - 1
                    threadControlBlocks[i].isAlive = 1;
                    threadControlBlocks[i].previousTCB = &threadControlBlocks[i - 1];
                    
                    // find next live thread
                    uint16_t j = i + 1;
                    j = (j == MAX_THREADS) ? 0 : j;
                    // wait until live thread found (there's gotta be one if this isn't the first thread!)
                    while (!threadControlBlocks[j].isAlive) {
                        // If j is equal to max threads, loop back to 0.
                        j++;
                        j = (j == MAX_THREADS) ? 0 : j;
                    }

                    threadControlBlocks[i].nextTCB = &threadControlBlocks[j];
                    threadControlBlocks[j].previousTCB = &threadControlBlocks[i];

                    threadControlBlocks[i].previousTCB = &threadControlBlocks[i - 1];
                    threadControlBlocks[i - 1].nextTCB = &threadControlBlocks[i];

                    threadIndex = i;
                    break;
                }
            }
        }

        threadControlBlocks[threadIndex].ThreadID = threadID++;
        threadControlBlocks[threadIndex].asleep = 0;
        threadControlBlocks[threadIndex].blocked = 0;
        threadControlBlocks[threadIndex].sleepCount = 0;
        threadControlBlocks[threadIndex].priority = priority;

        int i = 0;
        while (name[i] != '\0' && i < MAX_NAME_LENGTH) {
            threadControlBlocks[threadIndex].Threadname[i] = name[i];
            i++;
        }
        name[i] = '\0';

        // Set up the stack pointer
        threadControlBlocks[threadIndex].stackPointer = &threadStacks[threadIndex][STACKSIZE - 16];
        threadStacks[threadIndex][STACKSIZE - 1] = THUMBBIT;
        threadStacks[threadIndex][STACKSIZE - 2] = (uint32_t)threadToAdd; // address to start of function

        // Increment number of threads present in the scheduler
        NumberOfThreads++;
    }

    EndCriticalSection(IBit_State);
    return NO_ERROR;
}



/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddPeriodicEvent(void (*PthreadToAdd)(void), uint32_t period, uint32_t execution)
{
    // your code
    if (NumberOfPthreads > MAXPTHREADS) {
        return THREAD_LIMIT_REACHED;
    } else {
        if (NumberOfPthreads == 0) {
            Pthread[0].nextPTCB = &(Pthread[0]);
            Pthread[0].previousPTCB = &(Pthread[0]);
        } else {
            Pthread[NumberOfPthreads].nextPTCB = &Pthread[0];
            Pthread[0].previousPTCB = &Pthread[NumberOfPthreads];

            Pthread[NumberOfPthreads].previousPTCB = &Pthread[NumberOfPthreads-1];
            Pthread[NumberOfPthreads-1].nextPTCB = &Pthread[NumberOfPthreads];
        }

            Pthread[NumberOfPthreads].handler = PthreadToAdd;
            Pthread[NumberOfPthreads].period = period;
            Pthread[NumberOfPthreads].executeTime = execution;

            // Increment number of threads present in the scheduler
            NumberOfPthreads++;
    }

    return NO_ERROR;
}



/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void sleep(uint32_t durationMS)
{
    CurrentlyRunningThread->sleepCount = durationMS + SystemTime;   //Sets sleep count
    CurrentlyRunningThread->asleep = 1;                             //Puts the thread to sleep
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;                  //Start context switch
}

threadId_t G8RTOS_GetThreadId()
{
    return CurrentlyRunningThread->ThreadID;        //Returns the thread ID
}

sched_ErrCode_t G8RTOS_KillThread(threadId_t threadID)
{
    IBit_State = StartCriticalSection();
    // Loop through tcb. If not found, return thread does not exist error. If there is only one thread running, don't kill it.
    // your code
    if (GetNumberOfThreads() == 1) {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    } else {
        tcb_t* currThread = CurrentlyRunningThread;

        if (currThread->ThreadID == threadID) {
            currThread->isAlive = false;
            G8RTOS_SignalSemaphore(currThread->blocked);
            currThread->previousTCB->nextTCB = currThread->nextTCB;
            currThread->nextTCB->previousTCB = currThread->previousTCB;
            NumberOfThreads--;
            HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
            EndCriticalSection(IBit_State);
            return NO_ERROR;
        } else {
            currThread = currThread->nextTCB;
            for (int i = 1; i < NumberOfThreads; i++) {
                if (currThread->ThreadID == threadID) {
                    currThread->isAlive = false;
                    currThread->previousTCB->nextTCB = currThread->nextTCB;
                    currThread->nextTCB->previousTCB = currThread->previousTCB;
                    G8RTOS_SignalSemaphore(currThread->blocked);
                    NumberOfThreads--;
                    return NO_ERROR;
                }
                currThread = currThread->nextTCB;
            }
        }

        EndCriticalSection(IBit_State);
        return THREAD_DOES_NOT_EXIST;
    }
}

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn) {
    if (priority > 6) {
        return HWI_PRIORITY_INVALID;
    }

    if (IRQn <= 0 && IRQn <= 155) {
        return IRQn_INVALID;
    }


    IntRegister(IRQn, AthreadToAdd);
    IntPrioritySet(IRQn, priority);
    return NO_ERROR;

}

//Thread kills itself
// Can only be called by currently running thread?
sched_ErrCode_t G8RTOS_KillSelf()
{
    // your code
    IBit_State = StartCriticalSection();

    if (GetNumberOfThreads() == 1) {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }
    NumberOfThreads--;
    CurrentlyRunningThread->isAlive = false;
    CurrentlyRunningThread->previousTCB->nextTCB = CurrentlyRunningThread->nextTCB;
    CurrentlyRunningThread->nextTCB->previousTCB = CurrentlyRunningThread->previousTCB;
    G8RTOS_SignalSemaphore(CurrentlyRunningThread->blocked);
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    EndCriticalSection(IBit_State);


    return NO_ERROR;
}

uint32_t GetNumberOfThreads(void)
{
    return NumberOfThreads;         //Returns the number of threads
}

void G8RTOS_KillAllThreads()
{
    IBit_State = StartCriticalSection();
    // your code
    for (int i = 0; i < MAX_THREADS; i++) {
        threadControlBlocks[i].isAlive = false;

        G8RTOS_SignalSemaphore(threadControlBlocks[i].blocked);
    }
    NumberOfThreads = 0;
    EndCriticalSection(IBit_State);
    return;
}






/*********************************************** Public Functions *********************************************************************/
