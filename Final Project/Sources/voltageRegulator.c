/*! @file voltageRegulator.c
 * 
 *  @brief Functions for checking VRMS and definite and inverse mode
 *  
 *  @author 13181680
 *  @date 6 Jun 2018
 */

 /*!
  *  @addtogroup voltageRegulator_module voltageRegulator module documentation
  *  @{
  */

#include "types.h"
#include "analog.h"
#include "PIT.h"
#include "Calculations.h"
#include "OS.h"
#include "Signal.h"
#include "voltageRegulator.h"
#include "analogMeasure.h"
#include "Flash.h"

#define UPPERBOUND 9803.1 // Analog value of upper bound
#define LOWERBOUND 6535.4 // Analog value of lower bound

int16_t newVRMS;

extern int8_t NumberOfRaises;
extern int8_t NumberOfLowers;

extern int8_t* NRaises;
extern int8_t* NLowers;

int16_t completedPercentage;
channelSample samples [3];

int checkChannel;
int alreadyChecking;
extern int8_t timingMode;
bool channelChecked = false;

//---------------- Inverse Time Mode variables ----------------//

float dev;
float oldDev;
float elapsedTime; // Time in milliseconds
float elapsedPercentage;
float globalTime;

/*! @brief Checks if the value of the VRMS is in or out of bounds.
 *  
 *  @param int16_t VRMS Value of the VRMS
 *  @param channelNb Number of the channel which is being checked
 *  @return void
 */
void vrmsChecker(int16_t VRMS, int channelNb)
{
  if ((VRMS > UPPERBOUND || VRMS < LOWERBOUND) && !channelChecked)
  {
    channelChecked = true;
    signalSetAlarm();
    checkChannel = channelNb;

    if (timingMode == 1)
    {
      definiteMode();
    }
    else if (timingMode == 2)
    {
      inverseMode();
    }
  }
  if ((VRMS < UPPERBOUND && VRMS > LOWERBOUND) && channelChecked && checkChannel == channelNb)
  {
    channelChecked = false;
    PIT_Enable1(false);
    for (int i = 0;  i < 3; i++)
    {
      if (samples[i].vrmsValue > UPPERBOUND || samples[i].vrmsValue < LOWERBOUND)
      {
        // Clear global variables
        dev = 0;
        oldDev = 0;
        elapsedTime = 0;
        elapsedPercentage = 0;
        globalTime = 0;

        return; // If any of the other channels still out of boundaries, keep signals set
      }
    }
    signalClearAlarm();
    signalClearHigh();
    signalClearLow();

    // Clear global variables
    dev = 0;
    oldDev = 0;
    elapsedTime = 0;
    elapsedPercentage = 0;
    globalTime = 0;
  }
}

/*! @brief Sets a PIT for 5s to be used in definiteCheck.
 *  
 *  @return void
 */
void definiteMode(void)
{
  PIT_Set1(5000000000, false); // Delay of 5 seconds in definite mode
}

/*! @brief Sets a PIT for 10ms to be used in inverseTimeMode.
 *  
 *  @return void
 */
void inverseMode(void)
{
  PIT_Set1(10000000, false); // PIT1 set to 10ms
}

/*! @brief Check of boundaries in definite mode.
 *  
 *  @return void
 */
void definiteCheck(void)
{
  newVRMS = samples[checkChannel].vrmsValue;
  if (newVRMS > UPPERBOUND)
  {
    signalSetLow();
    NumberOfLowers++;
    // Flash_Write8((int8_t* )NLowers, NumberOfLowers);
  }
  else if (newVRMS < LOWERBOUND)
  {
    signalSetHigh();
    NumberOfRaises++;
    // Flash_Write8((int8_t* )NRaises, NumberOfRaises);
  }
  else
  {
    signalClearAlarm();
    signalClearHigh();
    signalClearLow();
  }
}

/*! @brief Check of boundaries in inverse mode.
 *  
 *  @return void
 */
void inverseTimeMode(void)
{
  if (samples[checkChannel].vrmsValue > UPPERBOUND)
  {
    if (elapsedPercentage < 100)
    {
      dev = (samples[checkChannel].vrmsValue - 8169.5) / 3267.7; // Deviation from medium value
      oldDev = dev;
      globalTime = (5 / (2 * oldDev)) * 1000; // Provides the time in milliseconds
      if (globalTime < 1000)
      {
        globalTime = 1000; // If the delay from the formula is lower than 1 second, make it 1 (statement constraint)
      }
      elapsedTime += 10; // Elapsed time increased in 10 milliseconds
      elapsedPercentage = (100 * elapsedTime / globalTime);
    }

    else
    {
      signalSetLow();
      NumberOfLowers++;
//      Flash_Write8((int8_t* )NLowers, NumberOfLowers);

      // Value reset

      dev = 0;
      oldDev = 0;
      elapsedTime = 0; // Time in milliseconds
      elapsedPercentage = 0;
      globalTime = 0;
    }
  }

  else if (samples[checkChannel].vrmsValue < LOWERBOUND)
  {
    if (elapsedPercentage < 100)
    {
      dev = (8169.5 - samples[checkChannel].vrmsValue) / 3267.7; // Deviation from medium value
      oldDev = dev;
      globalTime = (5 / (2 * oldDev)) * 1000; // Provides the time in milliseconds
      if (globalTime < 1000)
      {
        globalTime = 1000; // If the delay from the formula is lower than 1 second, make it 1 (statement constraint)
      }
      elapsedTime += 10;
      elapsedPercentage = (100 * elapsedTime / globalTime);
    }
    else
    {
      signalSetHigh();
      NumberOfRaises++;
//      Flash_Write8((int8_t* )NRaises, NumberOfRaises);

      // Value reset

      dev = 0;
      oldDev = 0;
      elapsedTime = 0; // Time in milliseconds
      elapsedPercentage = 0;
      globalTime = 0;
    }
  }
  else
  {
    PIT_Enable1(false);
    signalClearHigh();
    signalClearLow();
    signalClearAlarm();

    // Value reset

    dev = 0;
    oldDev = 0;
    elapsedTime = 0; // Time in milliseconds
    elapsedPercentage = 0;
    globalTime = 0;
  }
}

/*!
 * @}
*/
