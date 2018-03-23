/*
 * FIFO.c
 *
 *  Created on: 23 Mar 2018
 *      Author: 13181680
 */

#include "FIFO.h"

bool FIFO_Put(TFIFO tfifo, /*Add data here*/){

  // Case: The buffer is full --> Start and End equal and different from 0
  if(tfifo.Start==tfifo.End){
      if(tfifo.Buffer[tfifo.Start]!=0){
	  // The array is full
	  return false;
      }
  }

  // Case: Start position is different from End position
  else if(tfifo.Start!=FIFO_SIZE){
      // Update the value of the End position with the new data
      tfifo.Buffer[tfifo.End] = Data;
      // Update the value of End position to the next one
      tfifo.End++;
      // Value properly added and position updated
      return true;
  }
  // Case: Start position is equal to End position
  else{
      // Update the value of the End position with the new data
      tfifo.Buffer[tfifo.End] = Data;
      // Update the value of the End position to the first one
      tfifo.End = 0;
      // Value properly added and position updated
      return true;
  }

}

bool FIFO_Get(TFIFO tfifo, ){

  // We update as the oldest position the following to the one consumed

  // In this case, as positions match and the content is equal to 0, the buffer is completely empty
  if(tfifo.Start == tfifo.End){
      if(tfifo.Buffer[tfifo.Start]==0){
	  //Empty buffer
	  return false;
      }
  }

  // If the oldest position is the final one of the array
  else if(tfifo.Start == FIFO_SIZE){
      tfifo.Start = 0;
      return true;
  }

  // If it is any other position of the array
  else{
      tfifo.Start++;
      return true;
  }

}






