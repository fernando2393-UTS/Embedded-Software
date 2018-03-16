/*! @file
 *
 *  @brief Routines to implement a simple timer using the LPTMR.
 *
 *  @author PMcL
 *  @date 2018-03-16
 */

/*!
 *  @addtogroup Timer_module Timer module documentation
 *  @{
*/

#include "IO_Map.h"
#include "PE_Types.h"
#include "Cpu.h"

// Simple Timer
#include "Timer.h"

static void (*UserFunction)(void *);  /*!< The user callback function. */
static void * UserArguments;          /*!< The user callback function arguments. */


void Timer_Init(void (*userFunction)(void*), void* userArguments)
{
  // Disable interrupts
  __DI();

  // Disable SysTick
  SYST_CSR = 0;

  // Set reload value for a 1/4 second period - the value must fit into the 24-bit SysTick counter
  SYST_RVR = CPU_CORE_CLK_HZ / 4 - 1;

  // Clear current value as well as count flag
  SYST_CVR = 0;

  // Enable SysTick, enable SysTick exception and use processor core clock
  SYST_CSR = (SysTick_CSR_ENABLE_MASK | SysTick_CSR_TICKINT_MASK | SysTick_CSR_CLKSOURCE_MASK);

  // Set up user callback function
  UserFunction = userFunction;
  UserArguments = userArguments;

  // Enable interrupts
  __EI();
}

void __attribute__ ((interrupt)) Timer_TickISR(void)
{
  static uint8_t Count = 0;
  
  Count++;
  if (Count == 4)
  {
    Count = 0;

    // Call user function
    if (UserFunction)
      (*UserFunction)(UserArguments);
  }
}

/*!
 * @}
*/
