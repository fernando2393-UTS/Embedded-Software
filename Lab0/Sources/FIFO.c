/*
 * FIFO.c
 *
 *  Created on: 16 Mar 2018
 *      Author: 13181680
 */

#include "stdint.h"
#include "FIFO.h"

uint16_t Array[10];

uint8_t FIFO_Write(Time_struct dataIn, uint8_t i){

  uint16_t timeStamp;
  // need to convert Time_struct into uint16_t

  uint8_t counter = 0;

  // need to convert Time_struct into uint16_t

  timeStamp = dataIn.seconds;
  timeStamp = timeStamp + (dataIn.minutes << 6);
  timeStamp = timeStamp + (dataIn.hours << 12);


  Array[i] == timeStamp;
  if(i==9){
      // If the counter is equal 10, restart as a circular array
      i = 0;
  }
  else{
      i++;
  }

  return i;
}

/*uint16_t FIFO_Give(void){

}*/


