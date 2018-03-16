/*
 * FIFO.c
 *
 *  Created on: 16 Mar 2018
 *      Author: 13181680
 */

uint16_t Array[10];
uint8_t counter;

uint8_t FIFO_Write(uint16_t dataIn){

  if(counter==10){
      return 1;
  }
  else{
      counter++;
      return 0;
  }
}

uint16_t FIFO_Give(void){

}


