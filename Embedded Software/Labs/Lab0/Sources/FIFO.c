/*
 * FIFO.c
 *
 *  Created on: 16 Mar 2018
 *      Author: 13181680
 */

uint16_t Array[10];

uint8_t FIFO_Write(Time_struct dataIn){

  uint16_t = timeStamp;
  // need to convert Time_struct into uint16_t

      uint8_t counter = 0;

      // need to convert Time_struct into uint16_t

      timeStamp = dataIn.seconds;
      timeStamp = timeStamp + (dataIn.minutes << 6);
      timeStamp = timeStamp + (dataIn.hours << 12);

      //We add here the whole number to the array
      Array[counter] = timeStamp;

      for(int i = 0; i<10; i++){
	  if(Array[i]==0){
	      Array[i]==timeStamp;
	  }
	  else{
	      counter++;
	  }
      }

      if(counter==10){
	  // If the counter is equal 10, no free positions remaining
	  return 1;
      }

      return 0;
  }

uint16_t FIFO_Give(void){

}


