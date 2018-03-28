/* ###################################################################
**     Filename    : main.c
**     Project     : Lab1
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 1.0
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


// CPU mpdule - contains low level hardware initialization routines
#include "Cpu.h"
#include "UART.h"
#include "packet.h"
#include "FIFO.h"

#define BAUDRATE 38400

void startPackets();
void packetHandler();

uint8_t LSB = 0x50; // LSB tower number
uint8_t MSB = 0x10; // MSB tower number

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  Packet_Init(BAUDRATE, CPU_BUS_CLK_HZ); // Initialization function

  startPackets(); //Initial packets

  for (;;)
  {
      UART_Poll();
      if(Packet_Get()){ //Check if received packets
	  packetHandler(); // Handle received packets
      }
  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */


void startPackets(){
  Packet_Put(0x04,0x00,0x00,0x00); // Tower startup packet
  Packet_Put(0x09,'v',1,0); // Tower version packet
  Packet_Put(0x0B,0x01,LSB,MSB); // Tower number packet
}

void packetHandler(){

  bool values = false;

  switch (Packet_Command & 0x7F)
  {

    case 0x04:

      // Here we get the startup values
      values = Packet_Put(0x04,0x00,0x00,0x00) & Packet_Put(0x09,'v',1,0) & Packet_Put(0x0B,0x01,LSB,MSB);

      break;

    case 0x09:

      values = Packet_Put(0x09,'v',1,0); // Version value

      break;

    case 0x0B:

      if (Packet_Parameter1 == 0x01){ // Get tower number
	  values = Packet_Put(0x0B,0x01,LSB,MSB);
      }

      if (Packet_Parameter1 == 0x02){ // Set tower number
	  LSB = Packet_Parameter2; // New LSB written
	  MSB = Packet_Parameter3; // New MSB written
	  values = Packet_Put(0x0B,0x01,LSB,MSB); // New tower number set
      }

      break;

    default: break;
  }

  if (Packet_Command & 0x80) // Check for ACK
     {
       if (values==false)
       {
         Packet_Command &= 0x7F; // Clear ACK flag
       }
       Packet_Put(Packet_Command,Packet_Parameter1,Packet_Parameter2,Packet_Parameter3);
     }

}



/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
