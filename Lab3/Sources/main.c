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
#include "types.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "LEDs.h"
#include "packet.h"
#include "FIFO.h"
#include "Flash.h"
#include "UART.h"
#include "FTM.h"
#include "RTC.h"
#include "PIT.h"


static const int BAUDRATE = 115200;

// Protocol packet definitions
#define CMD_STARTUP 0x04
#define CMD_WRITEBYTE 0x07
#define CMD_READBYTE 0x08
#define CMD_TWRVERSION 0x09
#define CMD_TWRNUMBER 0x0B
#define CMD_TIME 0x0C
#define CMD_TWRMODE 0x0D
#define ACK_REQUEST_MASK 0x80

void startPackets();
void packetHandler();
void PITCallback(void* arg);
void RTCCallback(void* arg);
void FTM0Callback(const TFTMChannel* const aTimer);
bool StartUp();
bool WriteByte();
bool ReadByte();
bool Version();
bool Number();
bool Time();
bool Mode();

// 1 second timer on FTM0 channel 0
TFTMChannel FTM_Timer = {0, CPU_MCGFF_CLK_HZ_CONFIG_0, TIMER_FUNCTION_OUTPUT_COMPARE, TIMER_OUTPUT_HIGH, (void* )&FTM0Callback, &FTM_Timer};

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

        __DI();

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
//      if(NTowerNumber->s.Lo == 0xFF && NTowerNumber->s.Hi == 0xFF)
                if(NTowerNumber->l == 0xFFFF)
                {
                        Flash_Write16((uint16_t* )NTowerNumber, TowerNumber);
                }
        }

        if(Flash_AllocateVar((void* )&NTowerMode, sizeof(*NTowerMode)))
        {
                Flash_Write16((uint16_t* )NTowerMode, TowerMode);
        }

        RTC_Init(&RTCCallback,NULL); // Initialize RTC
        PIT_Init(CPU_BUS_CLK_HZ,&PITCallback,NULL); // Initialize PIT0
        PIT_Set(500000000,true); // Store PIT0 with a period of 0.5 seconds
        FTM_Init();
        FTM_Set(&FTM_Timer); // Setup FTM0 timer

        __EI();

        startPackets();

        for (;;)
        {
                if(Packet_Get()) //Check for received packets from PC
                {
                        packetHandler(); //If received execute packet handler
                }
        }

        /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
        /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
        PEX_RTOS_START();              /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
        /*** End of RTOS startup code.  ***/
        /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
        for(;;) {}
        /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/




/* END main */


/*! @brief Puts in the buffer four packets with initial values
 */
void startPackets()
{
        Packet_Put(0x04, 0x00, 0x00, 0x00); //Tower startup packet
        Packet_Put(0x09,'v',1,0); //Tower version number
        Packet_Put(0x0B,1,NTowerNumber->s.Lo,NTowerNumber->s.Hi); // Tower number packet
        Packet_Put(0x0D,1,NTowerMode->s.Lo,NTowerNumber->s.Hi); // Tower mode packet
}


/*! @brief Handle function for received packets
 */
void packetHandler()
{

        bool values = false;

        LEDs_On(LED_BLUE); // Turn on blue LED
        FTM_Timer.ioType.inputDetection = TIMER_OUTPUT_HIGH; // Start FTM0 timer
        FTM_StartTimer(&FTM_Timer);

        switch(Packet_Command & 0x7F)
        {


        case CMD_STARTUP:

                // Here we get the startup values
                values = StartUp();
                break;


        case CMD_TWRVERSION:

                values = Version(); // Version value

                break;


        case CMD_TWRNUMBER:

                values = Number();

                break;


        case CMD_WRITEBYTE:

                values = WriteByte();

                break;


        case CMD_READBYTE:

                values = ReadByte();

                break;


        case CMD_TWRMODE:

                values = Mode();

                break;


        case CMD_TIME:

                values = Time();

                break;

        default: break;
        }

        if (Packet_Command & ACK_REQUEST_MASK) // Check for ACK
        {
                if (values==false)
                {
                        Packet_Command &= 0x7F; // Clear ACK flag
                }
                Packet_Put(Packet_Command,Packet_Parameter1,Packet_Parameter2,Packet_Parameter3);
        }

}



/*! @brief User callback function for PIT
 *
 *  @param arg Pointer to the user argument to use with the user callback function
 *  @note Assumes PIT interrupt has occurred
 */
void PITCallback(void* arg)
{
        if (PIT_TFLG0 & PIT_TFLG_TIF_MASK) // Check if timeout has occurred (500 ms)
        {
                PIT_TFLG0 |= PIT_TFLG_TIF_MASK; // Clear timer interrupt flag
                LEDs_Toggle(LED_GREEN); // Toggle green LED
        }
}

/*! @brief User callback function for RTC
 *
 *  @param arg Pointer to the user argument to use with the user callback function
 *  @note Assumes RTC interrupt has occurred
 */
void RTCCallback(void* arg)
{
        uint8_t hours, minutes, seconds; /*!< Variables to store current time */

        RTC_Get(&hours,&minutes,&seconds); // Get current time each second
        Packet_Put(CMD_TIME,hours,minutes,seconds); // Update time in PC
        LEDs_Toggle(LED_YELLOW); // Toggle yellow LED
}


/*! @brief User callback function for FTM0
 *
 *  @param aTimer Structure containing the parameters used for setting up the timer channel
 *  @note Assumes FTM0 interrupt has occurred
 */
void FTM0Callback(const TFTMChannel* const aTimer)
{
        LEDs_Off(LED_BLUE); // Turn off blue LED
}

bool StartUp()
{
  bool success;
  success = Packet_Put(0x04,0x00,0x00,0x00) && Packet_Put(0x09,'v',1,0) && Packet_Put(0x0B,0x01,NTowerNumber->s.Lo,NTowerNumber->s.Hi) && Packet_Put(0x0D,0x01,NTowerMode->s.Lo,NTowerMode->s.Hi);
  return success;
}

bool WriteByte(void)
{

  bool success;

  if(Packet_Parameter1 > 7)
  {
          Flash_Erase();
          return success;
  }

  else
  {
          success = Flash_Write8((uint8_t*) (FLASH_DATA_START+Packet_Parameter1), Packet_Parameter3);
          return success;
  }
}

bool ReadByte(void)
{
  bool success;

  success = Packet_Put(0x08, Packet_Parameter1, 0, _FB(FLASH_DATA_START+Packet_Parameter1));
  return success;
}

bool Version(void)
{
  bool success;
  success = Packet_Put(0x09,'v',1,0);
  return success;
}

bool Number(void)
{

  bool success;

  if (Packet_Parameter1 == 0x01) { // Get tower number
          success = Packet_Put(0x0B,0x01,NTowerNumber->s.Lo,NTowerNumber->s.Hi);
          return success;
  }

  else if (Packet_Parameter1 == 0x02)
  { // Set tower number
          if(Flash_AllocateVar((void*)&NTowerNumber, sizeof(*NTowerNumber)))
          {
                  success = Flash_Write16((uint16_t*) NTowerNumber, Packet_Parameter23);
                  return success;
          }
  }
}

bool Time(void)
{
  bool success;

  RTC_Set(Packet_Parameter1,Packet_Parameter2,Packet_Parameter3);
  success = true;
  return success;
}

bool Mode(void)
{

  bool success;

  if(Packet_Parameter1 == 1) //Get tower mode
  {
          success = Packet_Put(0x0D, 1, NTowerMode->s.Lo, NTowerMode->s.Hi);
          return success;
  }

  else if(Packet_Parameter1 == 2)
  {
          if(Flash_AllocateVar((void*)&NTowerMode, sizeof(*NTowerMode)))
          {
                  success = Flash_Write16((uint16_t*) NTowerMode, Packet_Parameter23);
                  return success;
          }
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
