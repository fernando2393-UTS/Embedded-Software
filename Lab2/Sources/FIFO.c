/*! @file
 *
 * @brief This file contents the implementation of the FIFO routines
 *
 *  Created on: 28 Mar 2018
 *      Author: 13181680
 */


#include "FIFO.h"


/*! @brief Initialize the FIFO before first use.
 *
 *  @param FIFO A pointer to the FIFO that needs initializing.
 *  @return void
 */
void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->NbBytes = 0; // Initialize FIFO bytes to 0
  FIFO->Start = 0; // Initialize the head pointer to position 0
  FIFO->End = 0; // Initialize the tail pointer to position 0
}


/*! @brief Put one character into the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct where data is to be stored.
 *  @param data A byte of data to store in the FIFO buffer.
 *  @return bool - TRUE if data is successfully stored in the FIFO.
 */
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


/*! @brief Get one character from the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct with data to be retrieved.
 *  @param dataPtr A pointer to a memory location to place the retrieved byte.
 *  @return bool - TRUE if data is successfully retrieved from the FIFO.
 */
bool FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{

  //Case: Buffer empty

  if (FIFO->NbBytes == 0)
    {// Checking if buffer empty
   return false; // Return false -> Buffer Empty
    }

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
