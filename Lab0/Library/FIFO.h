/*
 * FIFO.h
 *
 *  Created on: 16 Mar 2018
 *      Author: 13181680
 */

#ifndef SOURCES_FIFO_H_
#define SOURCES_FIFO_H_

#include "stdint.h"

typedef struct {
  uint16_t hours;
  uint16_t minutes;
  uint16_t seconds;
}Time_struct;

uint8_t FIFO_Write(Time_struct dataIn, uint8_t i);




#endif /* SOURCES_FIFO_H_ */
