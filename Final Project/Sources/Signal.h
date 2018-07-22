/*! @file Signal.h
 *  
 *  @brief Definition of singals functions
 * 
 *  @author 13181680
 *  @date 9 Jun 2018
 */

#ifndef SOURCES_Signal_H_
#define SOURCES_Signal_H_

/*! @brief Set 5 Volts on channel 2.
 *
 *  @return void
 */
void signalSetAlarm();

/*! @brief Set 0 Volts on channel 2.
 *
 *  @return void
 */
void signalClearAlarm();

/*! @brief Set 5 Volts on channel 0.
 *
 *  @return void
 */
void signalSetHigh();

/*! @brief Set 0 Volts on channel 0.
 *
 *  @return void
 */
void signalClearHigh();

/*! @brief Set 5 Volts on channel 1.
 *
 *  @return void
 */
void signalSetLow();

/*! @brief Set 0 Volts on channel 1.
 *
 *  @return void
 */
void signalClearLow();


#endif /* SOURCES_Signal_H_ */
