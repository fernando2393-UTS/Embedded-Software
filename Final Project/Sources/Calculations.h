/*! @file Calculations.h
 *
 *  @brief Definition of conversion functions
 * 
 *  @author 13181680
 *  @date 9 Jun 2018
 * 
 */

#ifndef SOURCES_CALCULATIONS_H_
#define SOURCES_CALCULATIONS_H_

/*! @brief It computes the VRMS from an array of samples.
 *
 *  @param Sample The array of 16 samples
 *  @return int16_t The value of the VRMS for the sample
 */
int16_t VRMS(int16_t Sample[]);

/*! @brief It converts voltage to analog format.
 *
 *  @param voltage The voltage measured
 *  @return int16_t The analog value of the input voltage
 */
int16_t VoltageToAnalog(int16_t value);


/*! @brief It converts analog value to voltage format.
 *
 *  @param analogValue The analog value measured
 *  @return int16_t The voltage corresponding to the input analog value
 */
int16_t AnalogToVoltage(int16_t value);

#endif /* SOURCES_CALCULATIONS_H_ */
