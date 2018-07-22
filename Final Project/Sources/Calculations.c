/*! @file Calculations.c
 *
 *  @brief Implementation of conversion functions
 * 
 *  @author 13181680
 *  @date 9 Jun 2018
 * 
 */ 
 
 /*!
  *  @addtogroup Calculations_module Calculations module documentation
  *  @{
  */

#include <math.h>
#include <types.h>
#include "OS.h"
#include "MK70F12.h"
#include "Calculations.h"

#define NB_OF_SAMPLE 16
#define BITS_PER_VOLT 3276.7  /* ie. each increment is an increase of 1V*/

/*! @brief It computes the VRMS from an array of samples.
 *
 *  @param Sample The array of 16 samples
 *  @return int16_t The value of the VRMS for the sample
 */
int16_t VRMS(int16_t Sample[NB_OF_SAMPLE])
{
  float v_rms;

  for (int i = 0; i<NB_OF_SAMPLE; i++)
  {
    v_rms += (Sample[i]) * (Sample[i]);
  }

  v_rms = v_rms/16;
  v_rms = sqrt(v_rms);


  return (int16_t) v_rms;
}

/*! @brief It converts analog value to voltage format.
 *
 *  @param analogValue The analog value measured
 *  @return int16_t The voltage corresponding to the input analog value
 */
int16_t AnalogToVoltage(int16_t value)
{
  int16_t aux = (int16_t) (value / BITS_PER_VOLT);
  return aux;
}


/*! @brief It converts voltage to analog format.
 *
 *  @param voltage The voltage measured
 *  @return int16_t The analog value of the input voltage
 */
int16_t VoltageToAnalog(int16_t value)
{
  int16_t aux = (int16_t) (value * BITS_PER_VOLT);
  return aux;
}

/*!
 * @}
*/