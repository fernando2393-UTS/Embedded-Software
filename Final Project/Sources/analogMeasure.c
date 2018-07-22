/*! @file
 *
 *  @brief Functions for performing the frequency tracking
 *
 *  analogMeasure.c
 *
 *  @author 13181680
 *  @date 6 Jun 2018
 */

 /*!
  *  @addtogroup analogMeasure_module analogMeasure module documentation
  *  @{
  */

#include "analog.h"
#include "types.h"
#include "stdio.h"
#include "stdlib.h"
#include "Calculations.h"
#include "PIT.h"
#include "analogMeasure.h"
#include "voltageRegulator.h"
#include "signal.h"
#include "math.h"
#include "kiss_fftr.h"



int16_t firstMin; // This is going to be the minimum one
int16_t secondMin; // This is going to be the next or previous value to the minimum one
int firstMinPosition; // Position in samples[0].myArray of minimum positive value
int secondMinPosition; // Position in samples[0].myArray of second minimum positive value
float finalPeriod; // Period value in float
int arrayPosition;
extern int32_t measurementsFreq;

extern float frequency;
channelSample samples [3];


/*! @brief Compares the new measured value with the ones already stored
 *
 *  @param value It is the new measured value
 *  @param position It is the position of the read value
 *  @return void
 */
void compareMinimum(float value, int position)
{
  if (value < firstMin)
  {
    secondMin = firstMin;
    firstMin = value;

    secondMinPosition = firstMinPosition;
    firstMinPosition = position;
  }
  else if (value > firstMin && value < secondMin)
  {
    secondMin = value;

    secondMinPosition = position;
  }
}


/*! @brief Calculates the time of a zero in the x axis
 *
 *  @param minimumPosition Position of the minimum value in the array
 *  @return int32 Value of the time in which voltage is 0  
 */
int32_t calculation (int minimumPosition)
{

  float crossing;
  float crossingtime;
  float minimum;
  float minimumAux;

  // Special case for position zero

  if (minimumPosition == 0)
  {
    if (samples[0].myArray[minimumPosition] < 0)
    {
      if (samples[0].myArray[minimumPosition + 1] > 0)
      {
        minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> negative
        minimumAux = samples[0].myArray[minimumPosition + 1]; // Consecutive value --> positive

        float divisor = abs(minimum) + abs(minimumAux);
        float auxdivision = (abs(minimum) / divisor);
        crossing = measurementsFreq * auxdivision;
        crossingtime = crossing;
        return (int32_t) crossingtime;
      }
    }
    if (samples[0].myArray[minimumPosition] > 0)
    {
      if (samples[0].myArray[minimumPosition + 1] < 0)
      {
        minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> positive
        minimumAux = samples[0].myArray[minimumPosition + 1]; // Next value --> negative

        float divisor = abs(minimum) + abs(minimumAux);
        float auxdivision = (abs(minimum) / divisor);
        crossing = measurementsFreq * auxdivision;
        crossingtime = crossing;
        return (int32_t) crossingtime;
      }
    }
    if (samples[0].myArray[minimumPosition] == 0)
    {
      crossing = 0;
      crossingtime = crossing;
      return (int32_t) crossingtime;
    }
  }

  // Special case for position 15

  else if (minimumPosition == 15)
    {
      if (samples[0].myArray[minimumPosition] < 0)
      {
        if (samples[0].myArray[minimumPosition - 1] > 0)
        {
          minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> negative
          minimumAux = samples[0].myArray[minimumPosition - 1]; // Previous value --> positive

          float divisor = abs(minimum) + abs(minimumAux);
          float auxdivision = (abs(minimum) / divisor);
          crossing = measurementsFreq * auxdivision;
          crossingtime = minimumPosition * measurementsFreq - crossing;
          return (int32_t) crossingtime;
        }
      }
      if (samples[0].myArray[minimumPosition] > 0)
      {
        if (samples[0].myArray[minimumPosition - 1] < 0)
        {
          minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> positive
          minimumAux = samples[0].myArray[minimumPosition - 1]; // Previous value --> negative

          float divisor = abs(minimum) + abs(minimumAux);
          float auxdivision = (abs(minimum) / divisor);
          crossing = measurementsFreq * auxdivision;
          crossingtime = minimumPosition * measurementsFreq - crossing;
          return (int32_t) crossingtime;
        }
      }
      if (samples[0].myArray[minimumPosition] == 0)
      {
        crossing = 0;
        crossingtime = crossing * measurementsFreq + measurementsFreq * minimumPosition;
        return (int32_t) crossingtime;
      }
    }

  // Remaining cases

  else
  {

    if (samples[0].myArray[minimumPosition] < 0)
    {
      if (samples[0].myArray[minimumPosition + 1] > 0)
      {
        minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> negative
        minimumAux = samples[0].myArray[minimumPosition + 1]; // Consecutive value --> positive

        float divisor = abs(minimum) + abs(minimumAux);
        float auxdivision = (abs(minimum) / divisor);
        crossing = measurementsFreq * auxdivision;
        crossingtime =  measurementsFreq * minimumPosition + crossing;
        return (int32_t) crossingtime;
      }

      if (samples[0].myArray[minimumPosition + 1] < 0)
      {
        minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> negative
        minimumAux = samples[0].myArray[minimumPosition - 1]; // Previous value --> positive

        float divisor = abs(minimum) + abs(minimumAux);
        float auxdivision = (abs(minimum) / divisor);
        crossing = measurementsFreq * auxdivision;
        crossingtime = measurementsFreq * minimumPosition - crossing;
        return (int32_t) crossingtime;
      }
    }

    if (samples[0].myArray[minimumPosition] > 0)
    {
      if (samples[0].myArray[minimumPosition + 1] > 0)
      {
        minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> positive
        minimumAux = samples[0].myArray[minimumPosition - 1]; // Previous value --> negative

        float divisor = abs(minimum) + abs(minimumAux);
        float auxdivision = (abs(minimum) / divisor);
        crossing = measurementsFreq * auxdivision;
        crossingtime = measurementsFreq * minimumPosition - crossing;
        return (int32_t) crossingtime;
      }

      if (samples[0].myArray[minimumPosition + 1] < 0)
      {
        minimum = samples[0].myArray[minimumPosition]; // Closest value to 0 --> positive
        minimumAux = samples[0].myArray[minimumPosition + 1]; // Next value --> negative

        float divisor = abs(minimum) + abs(minimumAux);
        float auxdivision = (abs(minimum) / divisor);
        crossing = measurementsFreq * auxdivision;
        crossingtime = measurementsFreq * minimumPosition + crossing;
        return (int32_t) crossingtime;
      }
    }

    if (samples[0].myArray[minimumPosition] == 0) // The value measured is exactly zero
    {
      crossing = 0;
      crossingtime = crossing * measurementsFreq + measurementsFreq * minimumPosition;
      return (int32_t) crossingtime;
    }
  }
}

/*! @brief Calculates the minimum values of the array sample and updates period and frequency
 *  @return void
 */
void calculateMinimum(void)
{
  for (int i = 0; i < (sizeof(samples[0].myArray)/sizeof(int16_t)); i++) // We check from second position to the previous of the last one
  {
    if (i==1) // We initialize firstMin to the second position of the array --> Enable to check minimum values
    {
      firstMin = abs(samples[0].myArray[i]);
      firstMinPosition = i;
    }
    else if (i==1) // We initialize secondMin to the second position of the array
    {
      secondMin = abs(samples[0].myArray[i]);
      secondMinPosition = i;
      if (secondMin < firstMin) // We check if secondMin is smaller than firstMin
      {
        int16_t aux;
        int auxpos;

        //Values update
        aux = firstMin;
        firstMin = secondMin;
        secondMin = aux;

        //Position update
        auxpos = firstMinPosition;
        firstMinPosition = secondMinPosition;
        secondMinPosition = auxpos;
      }
    }
    else
    {
      compareMinimum(abs(samples[0].myArray[i]), i); // New element value comparison
    }
  }

  // End of for loop --> The two minimums are already calculated, as well as their position

  int minimumPosition = firstMinPosition;
  int16_t minimum = firstMin;
  int32_t crossingTime1;

  crossingTime1 = calculation(firstMinPosition);

  minimumPosition = secondMinPosition;
  minimum = secondMin;
  int32_t crossingTime2;

  crossingTime2 = calculation(secondMinPosition);

  int32_t halfPeriod = abs(crossingTime2 - crossingTime1);
  if (halfPeriod == 0)
  {
    frequency = 50;
    measurementsFreq = 1250000;
  }
  else
  {
    int32_t fullPeriod = halfPeriod * 2;
    measurementsFreq = fullPeriod/16;
    float secondsPeriod = (float)fullPeriod / 1000000000; // Time in seconds
    frequency = 1/secondsPeriod;
  }

  if (frequency > 52.5 || frequency < 47.5)
  {
    frequency = 50; // Setting the frequency to 50 Hz
    measurementsFreq = 1250000; // Setting period to default value
  }
}

/*! @brief Calculates the values of the spectrum
 *  @param int16_t* samples Array of samples taken from channel one
 *  @param float* amplitude Amplitude of the wave
 * 
 *  @return void
 */
void FFT_Freq(int16_t* samples, float* amplitude)
{
  uint8_t memory[1024];
  size_t sizeMemory = sizeof(memory);
  kiss_fftr_cfg cfg = kiss_fftr_alloc (16, 0, memory, &sizeMemory);

  kiss_fft_scalar Inputs[16];
  kiss_fft_cpx Outputs[9]; // 16/2 + 1

  for (int i = 0; i < 16; i++)
  {
    Inputs[i] = (float)((*samples + i)/3276.7);
  }
  kiss_fftr (cfg, Inputs, Outputs);

  for (int i = 0; i < 9; i++)
  {
    *(amplitude + i) = sqrt(pow(Outputs[i].r, 2) + pow(Outputs[i].i, 2));
  }
}

/*!
 * @}
*/
