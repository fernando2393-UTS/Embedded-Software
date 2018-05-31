/*! @file
 *
 * @brief This file contents the implementation of the Periodic Interrupt Timer (PIT) functions
 *
 *  Created on: 25 Apr 2018
 *      Author: 13181680
 */

 /*!
  *  @addtogroup pit_module PIT module documentation
  *  @{
  */

#include "PIT.h"
#include "types.h"
#include "IO_Map.h"

void* UserArgumentsGlobal;           /*!< Private global pointer to the user arguments to use with the user callback function */
void (*PITCallbackGlobal)(void *);  /*!< Private global pointer to PIT user callback function */
static uint32_t ModuleClk_Global_MHz; /*!< Module clock value in MHz */


/*! @brief Sets up the PIT before first use.
 *
 *  Enables the PIT and freezes the timer when debugging.
 *  @param moduleClk The module clock rate in Hz.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the PIT was successfully initialized.
 *  @note Assumes that moduleClk has a period which can be expressed as an integral number of nanoseconds.
 */
bool PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{

  ModuleClk_Global_MHz = moduleClk / 1000000; //It converts the clock module to MHz
  UserArgumentsGlobal = userArguments; // userArguments made globally(private) accessible
  PITCallbackGlobal = userFunction; // userFunction made globally(private) accessible

  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK; //Enable PIT
  PIT_MCR &= ~PIT_MCR_FRZ_MASK; // Run timers when in debug mode (PIT_MCR = 0)
  PIT_MCR &= ~PIT_MCR_MDIS_MASK; // Enable clock for PIT

  NVICICPR2 = (1<<4); //Interrupt clear instruction
  NVICISER2 = (1<<4); //Interrupt enable instruction

  return true;
}


/*! @brief Sets the value of the desired period of the PIT.
 *
 *  @param period The desired value of the timer period in nanoseconds.
 *  @param restart TRUE if the PIT is disabled, a new value set, and then enabled.
 *                 FALSE if the PIT will use the new value after a trigger event.
 *  @note The function will enable the timer and interrupts for the PIT.
 */
void PIT_Set(const uint32_t period, const bool restart)
{
  if(restart == true)
  {
    PIT_Enable(false);
    PIT_LDVAL0 = ((period/1000) * ModuleClk_Global_MHz) -1; // Setup timer0 for 12500000 cycles
  }

  PIT_Enable(true);
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK; // Enable interrupts for PIT
}


/*! @brief Enables or disables the PIT.
 *
 *  @param enable - TRUE if the PIT is to be enabled, FALSE if the PIT is to be disabled.
 */
void PIT_Enable(const bool enable)
{
  if(enable == true) // Enable PIT
  {
    PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
  }
  else // Disable PIT
  {
    PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
  }
}


/*! @brief Interrupt service routine for the PIT.
 *
 *  The periodic interrupt timer has timed out.
 *  The user callback function will be called.
 *  @note Assumes the PIT has been initialized.
 */
void __attribute__ ((interrupt)) PIT_ISR(void)
{
  if(PITCallbackGlobal)
  {
    (*PITCallbackGlobal)(UserArgumentsGlobal); // PIT ISR callback function
  }
}

/*!
 * @}
*/
