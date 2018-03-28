/*
 * FIFO.c
 *
 *  Created on: 28 Mar 2018
 *      Author: 13181680
 */


#include "FIFO.h"

void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->NbBytes = 0; // Initialize FIFO bytes to 0
  FIFO->Start = 0; // Initialize the head pointer to position 0
  FIFO->End = 0; // Initialize the tail pointer to position 0
}

bool FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{

  //Case: Buffer is full

  if (FIFO->NbBytes == FIFO_SIZE)  // Check if buffer is full
    return false; // Return false -> Buffer Full

  //Case: Buffer not full

  else
  {
    FIFO->Buffer[FIFO->End] = data; //Insert new data into buffer end position

    if (FIFO->End == FIFO_SIZE-1) // If end position is the last one, reset it to zero
      FIFO->End = 0;
    else
      FIFO->End++; // Move end pointer to next position

    FIFO->NbBytes++; // Increment number of bytes stored in buffer
      return true; // Byte inserted to FIFO successfully
  }
}

bool FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{

  //Case: Buffer empty

  if (FIFO->NbBytes == 0) // Checking if buffer empty
   return false; // Return false -> Buffer Empty

  //Case: Buffer not empty

  else
  {
    *dataPtr = FIFO->Buffer[FIFO->Start]; // Get oldest data in buffer

    if (FIFO->Start == FIFO_SIZE-1) // If start position is the last one, reset it to zero
      FIFO->Start = 0;
    else
      FIFO->Start++; // Move start pointer to next position

    FIFO->NbBytes--; // Reduce number of bytes stored in buffer
      return true; // Byte extracted from FIFO successfully
  }
}







