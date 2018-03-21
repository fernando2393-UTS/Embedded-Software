/*!
** @file
** @version 1.0
** @brief
**         Main module.
**         This module implements a simple 12-hour clock.
**         It time-stamps button pushes and stores them in a FIFO used a packed representation.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


// CPU module - contains low level hardware initialization routines
#include "Cpu.h"

// Simple timer
#include "timer.h"

// Button functions
#include "buttons.h"

// LED functions
#include "LEDs.h"

#include "FIFO.h"

// The packed time representation

//   15             12   11                        6    5                       0
// |----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
// |       hours       |          minutes            |          seconds            |

typedef uint16_t PackedTime_t;

// ***
// You will need to create a FIFO object with a size suitable to store 10 time-stamps using the packed time representation.
// ***

uint8_t Seconds = 0;
uint8_t Minutes = 0;
uint8_t Hours = 0;


static void OneSecondElapsed(void)
{
  LEDs_Toggle(LED_BLUE);
  // One second has elapsed - update the time here
  Seconds++;
  if(Seconds==60){
      Seconds = 0;
      Minutes++;
  }

  if(Minutes==60){
      Minutes = 0;
      Hours++;
  }

  if(Hours==12){
      Hours=0;
  }
}


uint8_t i = 0;
Time_struct timeToStore;

static void Button1Pressed(void)
{
  LEDs_Toggle(LED_ORANGE);
  // The button has been pressed - put a time-stamp into the FIFO
  //Seconds
  timeToStore.seconds = Seconds;
  // Minutes
  timeToStore.minutes = Minutes;
  // Hours
  timeToStore.hours = Hours;

  i = FIFO_Write(timeToStore, i);
}

static void TowerInit(void)
{
  PE_low_level_init();
  Timer_Init(OneSecondElapsed);
  Buttons_Init(Button1Pressed);
  LEDs_Init();
  __EI();
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{

  TowerInit();
  /* Write your code here */
  for (;;)
  {

  }
}

/* END main */
/*!
** @}
*/
