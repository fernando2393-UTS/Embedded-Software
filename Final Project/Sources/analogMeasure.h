/*! @file analogMeasure.h
 * 
 *  @brief Definition of the functions for frequency tracking
 *  
 *  @author 13181680
 *  @date 17 Jun 2018
 * 
 */

#ifndef SOURCES_ANALOGMEASURE_H_
#define SOURCES_ANALOGMEASURE_H_

#include "types.h"

/*! @brief Compares the new measured value with the ones already stored
 *
 *  @param value It is the new measured value
 *  @param position It is the position of the read value
 *  @return void
 */
void compareMinimum(float value, int position);

/*! @brief Calculates the time of a zero in the x axis
 *
 *  @param minimumPosition Position of the minimum value in the array
 *  @return int32 Value of the time in which voltage is 0  
 */
int32_t calculation (int minimumPosition);

/*! @brief Calculates the minimum values of the array sample and updates period and frequency
 *  @return void
 */
void calculateMinimum(void);

/*! @brief Calculates the values of the spectrum
 *  @param int16_t* samples Array of samples taken from channel one
 *  @param float* amplitude Amplitude of the wave
 * 
 *  @return void
 */
void FFT_Freq(int16_t* samples, float* amplitude);

#endif /* SOURCES_ANALOGMEASURE_H_ */
