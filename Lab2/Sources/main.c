/* ###################################################################
**     Filename    : main.c
**     Project     : Lab2
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
** @version 2.0
** @brief
**         Main module.
**         This module contains user's application code.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "LEDs.h"
#include "packet.h"
#include "FIFO.h"
#include "Flash.h"
#include "UART.h"


static const int BAUDRATE = 115200;

void startPackets();
void packetHandler();

uint16union_t* NTowerNumber;
uint16union_t* NTowerMode;

static uint16_t TowerNumber = 1680; //Last 4 digits of student ID (0x690)
static uint16_t TowerMode = 1; //Initial tower mode

TFIFO TxFIFO, RxFIFO;
TPacket Packet;


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  LEDs_Init();

  if(Flash_Init() && Packet_Init(BAUDRATE, CPU_BUS_CLK_HZ))
    {
      LEDs_On(LED_ORANGE);
    }

    // After allocate, you need to check if the data at the address is 0xFFFF
    // if it is, then you need to write in the "default value", such as your student number.
    // Otherwise, leave the data alone!!!!!!!!!!!!!
  if(Flash_AllocateVar((void* )&NTowerNumber, sizeof(*NTowerNumber)))
    {
      if(*NTowerNumber==0xFFFF)
      {
        Flash_Write16((uint16_t* )NTowerNumber, TowerNumber);
      }
    }

  if(Flash_AllocateVar((void* )&NTowerMode, sizeof(*NTowerMode)))
      {
        Flash_Write16((uint16_t* )NTowerMode, TowerMode);
      }

  startPackets();

  for (;;)
  {
      UART_Poll();
      if(Packet_Get()) //Check for received packets from PC
	{
	  packetHandler(); //If received execute packet handler
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

void startPackets()
{
  Packet_Put(0x04, 0x00, 0x00, 0x00); //Tower startup packet
  Packet_Put(0x09,'v',1,0); //Tower version number
  Packet_Put(0x0B,1,NTowerNumber->s.Lo,NTowerNumber->s.Hi); // Tower number packet
  Packet_Put(0x0D,1,NTowerMode->s.Lo,NTowerNumber->s.Hi); // Tower mode packet
}

void packetHandler()
{

  bool values = false;

  switch(Packet_Command & 0x7F){

    case 0x04:

      // Here we get the startup values
      values = Packet_Put(0x04,0x00,0x00,0x00) && Packet_Put(0x09,'v',1,0) && Packet_Put(0x0B,0x01,NTowerNumber->s.Lo,NTowerNumber->s.Hi) && Packet_Put(0x0D,0x01,NTowerMode->s.Lo,NTowerMode->s.Hi);

      break;

    case 0x09:

      values = Packet_Put(0x09,'v',1,0); // Version value

      break;

    case 0x0B:

      if (Packet_Parameter1 == 0x01){ // Get tower number
	  values = Packet_Put(0x0B,0x01,NTowerNumber->s.Lo,NTowerNumber->s.Hi);
      }

      else if (Packet_Parameter1 == 0x02){ // Set tower number
	  if(Flash_AllocateVar((void*)&NTowerNumber, sizeof(*NTowerNumber)))
	    {
	      values = Flash_Write16((uint16_t*) NTowerNumber, Packet_Parameter23);
	    }
      }

      break;

    case 0x07:

      if(Packet_Parameter1 > 7)
	{
	  Flash_Erase();
	}

      else
	{
	  values = Flash_Write8((uint8_t*) (FLASH_DATA_START+Packet_Parameter1), Packet_Parameter3);
	}

      break;

    case 0x08:

      values = Packet_Put(0x08, Packet_Parameter1, 0, _FB(FLASH_DATA_START+Packet_Parameter1));

      break;

    case 0x0D:

      if(Packet_Parameter1 == 1) //Get tower mode
	{
	  values = Packet_Put(0x0D, 1, NTowerMode->s.Lo, NTowerMode->s.Hi);
	}

      else if(Packet_Parameter1 == 2)
	{
	  if(Flash_AllocateVar((void*)&NTowerMode, sizeof(*NTowerMode)))
	  	    {
	  	      values = Flash_Write16((uint16_t*) NTowerMode, Packet_Parameter23);
	  	    }
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
