/*! @file
 *
 * @brief This file contents the implementation of the Real Time Clock (RTC) functions
 *
 *  Created on: 25 Apr 2018
 *      Author: 13181680
 */

 /*!
  *  @addtogroup rtc_module RTC module documentation
  *  @{
  */


#include "types.h"
#include "RTC.h"
#include "IO_Map.h"

static void* UserArgumentsGlobal;          /*!< Private global pointer to the user arguments to use with the user callback function */
static void (*RTCCallbackGlobal)(void *); /*!< Private global pointer to RTC user callback function */


/*! @brief Initializes the RTC before first use.
 *
 *  Sets up the control register for the RTC and locks it.
 *  Enables the RTC and sets an interrupt every second.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the RTC was successfully initialized.
 */
bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{

  UserArgumentsGlobal = userArguments; // userArguments made globally(private) accessible
  RTCCallbackGlobal = userFunction; // userFunction made globally(private) accessible

  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  RTC_CR = RTC_CR_SWR_MASK;
  RTC_CR &= ~RTC_CR_SWR_MASK;

  RTC_TSR = 0; //clear the time invalid flag

  RTC_CR |= RTC_CR_SC16P_MASK | RTC_CR_SC2P_MASK; // Enable 18pF load as seen in the schematic of the tower
  RTC_CR |= RTC_CR_OSCE_MASK; // Enable oscillator

  RTC_LR &= ~RTC_LR_CRL_MASK;         // Lock control register

  RTC_IER |= RTC_IER_TSIE_MASK;       // Enable time seconds interrupt
//  RTC_IER &= ~RTC_IER_TAIE_MASK;			// Disables Time Alarm Interrupt
//  RTC_IER &= ~RTC_IER_TOIE_MASK;			// Disables Overflow Interrupt
//  RTC_IER &= ~RTC_IER_TIIE_MASK;			// Disables Time Invalid Interrupt

  RTC_SR |= RTC_SR_TCE_MASK;				  // Initialises the timer control

  NVICICPR2 = (1<<3); // Clear pending interrupts on RTC module
  NVICISER2 = (1<<3); // Enable interrupts on RTC module

  return true;

}


/*! @brief Sets the value of the real time clock.
 *
 *  @param hours The desired value of the real time clock hours (0-23).
 *  @param minutes The desired value of the real time clock minutes (0-59).
 *  @param seconds The desired value of the real time clock seconds (0-59).
 *  @note Assumes that the RTC module has been initialized and all input parameters are in range.
 */
void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{

  RTC_SR &= ~RTC_SR_TCE_MASK; // Disable time counter
  RTC_TSR = (hours * 3600) + (minutes * 60) + seconds; // Put time value to time seconds register
  RTC_SR |= RTC_SR_TCE_MASK; //Re-enable time counter

}


/*! @brief Gets the value of the real time clock.
 *
 *  @param hours The address of a variable to store the real time clock hours.
 *  @param minutes The address of a variable to store the real time clock minutes.
 *  @param seconds The address of a variable to store the real time clock seconds.
 *  @note Assumes that the RTC module has been initialized.
 */
void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{

  uint32_t timeTemp = RTC_TSR; /*!< Temporarily stores time value in register */

  *hours = (timeTemp / 3600) % 24; // Update hours value
  timeTemp = timeTemp % 3600;
  *minutes = timeTemp / 60; // Update minutes value
  timeTemp = timeTemp % 60;
  *seconds = timeTemp; // Update seconds value

}

/*! @brief Interrupt service routine for the RTC.
 *
 *  The RTC has incremented one second.
 *  The user callback function will be called.
 *  @note Assumes the RTC has been initialized.
 */
void __attribute__ ((interrupt)) RTC_ISR(void)
{

  if(RTCCallbackGlobal)
  {
    (*RTCCallbackGlobal)(UserArgumentsGlobal);
  }

}

/*!
 * @}
*/
