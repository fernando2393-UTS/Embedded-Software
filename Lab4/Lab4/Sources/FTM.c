/*! @file FTM.c
 *
 *  @brief This file contents the initialization and implementation of the Flexible Timer Module (FTM)
 *
 *  @author 13181680 & 12099115
 *  @date 20 Apr 2018
 */


 /*!
  *  @addtogroup FTM documentation
  *  @{
  */

#include "FTM.h"
#include "types.h"
#include "IO_Map.h"

#define NumberOfChannels 8

void* UserArgumentsGlobal;          /*!< Private global pointer to the user arguments to use with the user callback function */
void (*FTM0CallbackGlobal)(void *); /*!< Private global pointer to FTM user callback function */


/*! @brief Sets up the FTM before first use.
 *
 *  Enables the FTM as a free running 16-bit counter.
 *  @return bool - TRUE if the FTM was successfully initialized.
 */
bool FTM_Init()
{ //For enable FTM we have to perform a value reset

  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK; //Enable FTM0 module

  FTM0_CNTIN = ~FTM_CNTIN_INIT_MASK; // Ensure the counter is a free running counter
  FTM0_MOD = FTM_MOD_MOD_MASK;
  FTM0_CNT = ~FTM_CNT_COUNT_MASK;

  FTM0_SC = FTM_SC_CLKS(2); // Use system fixed frequency clock for the counter (0x10)

  NVICICPR1 = (1<<30); //Interrupt clear instruction
  NVICISER1 = (1<<30); //Interrupt enable instruction

  return true;
}



/*! @brief Sets up a timer channel.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *    channelNb is the channel number of the FTM to use.
 *    delayCount is the delay count (in module clock periods) for an output compare event.
 *    timerFunction is used to set the timer up as either an input capture or an output compare.
 *    ioType is a union that depends on the setting of the channel as input capture or output compare:
 *      outputAction is the action to take on a successful output compare.
 *      inputDetection is the type of input capture detection.
 *    userFunction is a pointer to a user callback function.
 *    userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the timer was set up successfully.
 */
bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
  if(aFTMChannel->timerFunction == TIMER_FUNCTION_INPUT_CAPTURE)
  {
    FTM0_CnSC(aFTMChannel->channelNb) &= ~(FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK); // Set function for input capture (00)
  }

  else
  {
    FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;
    FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK; //Set function for output compare (01)
  }

  switch(aFTMChannel->ioType.inputDetection)
  {
    case 1: // inputDetection is TIMER_INPUT_RISING
         FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSB_MASK; // Set function for rising edge only (01)
	       FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSA_MASK;
         break;

    case 2: // inputDetection is TIMER_INPUT_FALLING
         FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSB_MASK; // Set function for falling edge only (10)
         FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSA_MASK;
         break;

    case 3: // inputDetection is TIMER_INPUT_ANY
         FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK; // Set function for any edge (11)
         break;

    default:
         FTM0_CnSC(aFTMChannel->channelNb) &= ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK); // Unused input capture (00)
	       break;
  }

  UserArgumentsGlobal = aFTMChannel->userArguments; // userArguments made globally(private) accessible
  FTM0CallbackGlobal = aFTMChannel->userFunction; // userFunction made globally(private) accessible

  return true;
}


/*! @brief Starts a timer if set up for output compare.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *  @return bool - TRUE if the timer was started successfully.
 */
bool FTM_StartTimer(const TFTMChannel* const aFTMChannel){

  if(aFTMChannel->channelNb < NumberOfChannels)
  {
    if(aFTMChannel->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
    {
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK; // Clear channel flag
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK; // Enable channel interrupts
      FTM0_CnV(aFTMChannel->channelNb) =  FTM0_CNT + (aFTMChannel->delayCount); // Set initial count

      return true; // Timer start successful
    }
  }
  return false;
}


/*! @brief Interrupt service routine for the FTM.
 *
 *  If a timer channel was set up as output compare, then the user callback function will be called.
 */
void __attribute__ ((interrupt)) FTM0_ISR(void)
{

  //Clear and disable interrupt inside the for loop

  for(int i = 0; i<NumberOfChannels; i++) //Check for all channels
  {

      FTM0_CnSC(i) &= ~FTM_CnSC_CHF_MASK; // Clear channel flag
      FTM0_CnSC(i) &= ~FTM_CnSC_CHIE_MASK; // Disable channel interrupts

    if(FTM0CallbackGlobal)
    {
      (*FTM0CallbackGlobal)(UserArgumentsGlobal);
    }
  }
}

/*!
 * @}
*/
