/*
 * FIFO.h
 *
 *  Created on: 16 Mar 2018
 *      Author: 13181680
 */

#ifndef SOURCES_FIFO_H_
#define SOURCES_FIFO_H_

  //Here we initialize an array of structures FIFO of maximum length 10

  FIFO array [FIFO_SIZE];

typedef struct {
  uint_16t hours;
  uint_16t minutes;
  uint_16t seconds;
}Time_struct;

uint8_t FIFO_Write(Time_struct dataIn);

Time_struct FIFO_Give(void);




#endif /* SOURCES_FIFO_H_ */
