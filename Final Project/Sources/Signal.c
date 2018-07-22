/*! @file Signal.c
 *
 *  @brief Functions to set alarm, raise and low, and clear them.
 *  
 *  @author 13181680
 *  @date 9 Jun 2018
 */

 /*!
  *  @addtogroup Signal_module Signal module documentation
  *  @{
  */

#include "analog.h"

/*! @brief Set 5 Volts on channel 2.
 *
 *  @return void
 */
void signalSetAlarm()
{
  Analog_Put(2,16335);
}

/*! @brief Set 0 Volts on channel 2.
 *
 *  @return void
 */
void signalClearAlarm()
{
  Analog_Put(2,0);
}

/*! @brief Set 5 Volts on channel 0.
 *
 *  @return void
 */
void signalSetHigh()
{
  Analog_Put(0,16335);
}

/*! @brief Set 0 Volts on channel 0.
 *
 *  @return void
 */
void signalClearHigh()
{
  Analog_Put(0,0);
}

/*! @brief Set 5 Volts on channel 1.
 *
 *  @return void
 */
void signalSetLow()
{
  Analog_Put(1,16335);
}

/*! @brief Set 0 Volts on channel 1.
 *
 *  @return void
 */
void signalClearLow()
{
  Analog_Put(1,0);
}

/*!
 * @}
*/