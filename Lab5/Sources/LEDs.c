/*! @file LEDs.c
 *
 *  @brief LEDs functions implementation and initialization
 *
 *  @author 13181680 & 12099115
 *  @date 6 Apr 2018
 */

 /*!
  *  @addtogroup LEDs_module LEDs module documentation
  *  @{
  */

#include "types.h"
#include "LEDs.h"
#include "MK70F12.h"

/*! @brief Sets up the LEDs before first use.
   *
   *  @return bool - TRUE if the LEDs were successfully initialized.
   */

bool LEDs_Init()
{

  PORTA_PCR10 |= PORT_PCR_MUX(1); //Pin 10 initialized ---> Blue
  PORTA_PCR11 |= PORT_PCR_MUX(1); //Pin 11 initialized ---> Orange
  PORTA_PCR28 |= PORT_PCR_MUX(1); //Pin 28 initialized ---> Yellow
  PORTA_PCR29 |= PORT_PCR_MUX(1); //Pin 29 initialized ---> Green

  GPIOA_PDDR |= LED_ORANGE | LED_BLUE | LED_YELLOW | LED_GREEN;

  //Turn on PortA

  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; //Port A enabled

  GPIOA_PSOR |= LED_ORANGE | LED_BLUE | LED_YELLOW | LED_GREEN;

}


/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 */

void LEDs_On(const TLED color)
{
  GPIOA_PCOR = color;
}


/*! @brief Turns off an LED.
  *
  *  @param color THe color of the LED to turn off.
  */

void LEDs_Off(const TLED color)
{
  GPIOA_PSOR = color;
}

/*! @brief Inverts LED's state ( ON to OFF and vice versa )
 *  @param color - toggled color
 */

void LEDs_Toggle(const TLED color)
{
  GPIOA_PTOR = color;
}

/*!
 * @}
*/
