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
#include "FIFO.h"
#include "Cpu.h"
#include "Events.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "stdint.h"

// Simple timer
#include "timer.h"

  // Definition of FIFO structure

typedef uint16_t PackedTime_t;

// ***
// You will need to create a FIFO object of size 10 to store time-stamps using the packed time representation.
// ***

uint8_t Seconds = 0;
uint8_t Minutes = 0;
uint8_t Hours = 0;



void OneSecondElapsed(void* arg)
{
  // One second has elapsed - update the time here
  Seconds = Seconds++;

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


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
  Time_struct timeToStore;


  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/
  PE_low_level_init();
  Timer_Init(OneSecondElapsed, NULL);

  /* Write your code here */
  for (;;)
  {
      // When Button is pushed
      // Execute this code
      // Seconds
      timeToStore.seconds = Seconds;
      // Minutes
      timeToStore.minutes = Minutes;
      // Hours
      timeToStore.hours = Hours;

      FIFO_Write(timeToStore);
  }
}

/* END main */
/*!
** @}
*/
