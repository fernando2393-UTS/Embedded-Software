/*! @file voltageRegulator.h
 * 
 *  @brief Definition of functions to check VRMS and implement timing modes
 *
 *  @author 13181680
 *  @date 18 Jun 2018
 */

#ifndef SOURCES_VOLTAGEREGULATOR_H_
#define SOURCES_VOLTAGEREGULATOR_H_

#include "types.h"

/*! @brief Checks if the value of the VRMS is in or out of bounds.
 *  
 *  @param int16_t VRMS Value of the VRMS
 *  @param channelNb Number of the channel which is being checked
 *  @return void
 */
void vrmsChecker(int16_t VRMS, int channelNb);

/*! @brief Check of boundaries in definite mode.
 *  
 *  @return void
 */
void definiteCheck(void);

/*! @brief Sets a PIT for 5s to be used in definiteCheck.
 *  
 *  @return void
 */
void definiteMode(void);

/*! @brief Sets a PIT for 10ms to be used in inverseTimeMode.
 *  
 *  @return void
 */
void inverseMode (void);

/*! @brief Check of boundaries in inverse mode.
 *  
 *  @return void
 */
void inverseTimeMode(void);

#endif /* SOURCES_VOLTSGEREGULATOR_H */
