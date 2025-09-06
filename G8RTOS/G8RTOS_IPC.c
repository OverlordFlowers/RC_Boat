/**
 * G8RTOS IPC - Inter-Process Communication
 * @author:
 * uP2 - Fall 2022
 */
#include <G8RTOS/G8RTOS_IPC.h>
#include <G8RTOS/G8RTOS_Semaphores.h>
#include <stdint.h>

#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */

/* Create FIFO struct here */

typedef struct FIFO_t {
    int32_t buffer[16];
    int32_t *head;
    int32_t *tail;
    uint32_t lostData;
    semaphore_t currentSize;
    semaphore_t mutex;
} FIFO_t;


/* Array of FIFOS */
static FIFO_t FIFOs[4];


/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
    // your code
    if (FIFOIndex >= MAX_NUMBER_OF_FIFOS) {
        return 1;
    } else {
        FIFOs[FIFOIndex].head = &(FIFOs[FIFOIndex].buffer[0]);
        FIFOs[FIFOIndex].tail = &FIFOs[FIFOIndex].buffer[0];
        FIFOs[FIFOIndex].lostData = 0;
        G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].currentSize, 0);
        G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].mutex, 1);

        return 0;
    }
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFOChoice)
{
    // your code
    if (FIFOChoice >= MAX_NUMBER_OF_FIFOS) {
        return INT32_MAX;
    } else {
        // Waits
        G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex);
        G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].currentSize);

        int32_t value = *FIFOs[FIFOChoice].head;

        // Increment head pointer
        *FIFOs[FIFOChoice].head++;
        // Wrap around
        if (FIFOs[FIFOChoice].head > &(FIFOs[FIFOChoice].buffer[0]) + (FIFOSIZE-1)) {
            FIFOs[FIFOChoice].head = &(FIFOs[FIFOChoice].buffer[0]);
        }

        // Reset mutex
        G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);

        return value;
    }

}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if necessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int32_t writeFIFO(uint32_t FIFOChoice, uint32_t Data)
{
    // your code
    if (FIFOChoice >= MAX_NUMBER_OF_FIFOS) {
        return 1;
    } else if (FIFOs[FIFOChoice].currentSize >= FIFOSIZE) {
        FIFOs[FIFOChoice].lostData++;
        return 2;
    } else {
        // overwrite old data with fresh data

        *FIFOs[FIFOChoice].tail = Data;

        FIFOs[FIFOChoice].tail++;
        if (FIFOs[FIFOChoice].tail > &(FIFOs[FIFOChoice].buffer[0]) + (FIFOSIZE-1)) {
            FIFOs[FIFOChoice].tail = &(FIFOs[FIFOChoice].buffer[0]);
        }

        G8RTOS_SignalSemaphore(&(FIFOs[FIFOChoice].currentSize));

        /*
        if ((FIFOs[FIFOChoice].currentSize) >= (FIFOSIZE)) {
            FIFOs[FIFOChoice].lostData++;
        } else {
            G8RTOS_SignalSemaphore(&(FIFOs[FIFOChoice].currentSize));
        }
        */
        return 0;
    }
}

