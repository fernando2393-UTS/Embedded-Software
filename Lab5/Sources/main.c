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
#include "accel.h"
#include "I2C.h"
#include "OS.h"

// Thread stack size - big enough for stacking of interrupts and OS use.
#define THREAD_STACK_SIZE 100

static const int BAUDRATE = 115200;

// Protocol packet definitions
#define CMD_STARTUP 0x04
#define CMD_WRITEBYTE 0x07
#define CMD_READBYTE 0x08
#define CMD_TWRVERSION 0x09
#define CMD_PROTOCOL 0x0A
#define CMD_ACCELVALUES 0x10
#define CMD_TWRNUMBER 0x0B
#define CMD_TIME 0x0C
#define CMD_TWRMODE 0x0D
#define ACK_REQUEST_MASK 0x80

static void InitThread(void* pData);
static void PacketCheckerThread(void* pData);
static void AccelReadyThread(void* pData);
static void I2CReadCompleteThread(void* pData);
static void RTCThread(void* pData);
static void PITThread(void* pData);
void StartPackets();
void PacketHandler();

bool StartUp();
bool WriteByte();
bool ReadByte();
bool Version();
bool Number();
bool Time();
bool Mode();
bool Protocol();

uint16union_t* NTowerNumber;
uint16union_t* NTowerMode;

static TFTMChannel FTMTimer0;          /*!< Stores content of 1 second timer on FTM0 channel 0 */

static uint16_t TowerNumber = 1680; //Last 4 digits of student ID (0x690)
static uint16_t TowerMode = 1; //Initial tower mode

TFIFO TxFIFO, RxFIFO;

volatile TAccelMode Protocol_Mode = ACCEL_POLL; /*!< Initial protocol mode selected */
static TAccelData accelerometerValues; /*!< Array to store accelerometer values */

TPacket Packet;

// Threads initialization

static uint32_t InitThreadStack[THREAD_STACK_SIZE];            /*!< The stack for the initialization thread. */
static uint32_t PacketCheckerThreadStack[THREAD_STACK_SIZE];   /*!< The stack for the packet checking thread. */
static uint32_t AccelReadyThreadStack[THREAD_STACK_SIZE];      /*!< The stack for the accelerometer data ready thread. */
static uint32_t I2CReadCompleteThreadStack[THREAD_STACK_SIZE]; /*!< The stack for the I2C read complete thread. */
static uint32_t RTCThreadStack[THREAD_STACK_SIZE];             /*!< The stack for the RTC thread. */
static uint32_t PITThreadStack[THREAD_STACK_SIZE];             /*!< The stack for the PIT thread. */

// Semaphores initialization

OS_ECB *Data_Ready_Semaphore;    /*!< Binary semaphore for signaling that data is ready to be read on accelerometer */
OS_ECB *Read_Complete_Semaphore; /*!< Binary semaphore for signaling that data was read successfully */
OS_ECB *Update_Clock_Semaphore;  /*!< Binary semaphore for signaling RTC update */
OS_ECB *PIT_Semaphore;           /*!< Binary semaphore for signaling PIT interrupt */


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  OS_ERROR error; /*!< Thread content */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  // Initialize the RTOS
  OS_Init(CPU_CORE_CLK_HZ, false);

  // Create threads using OS_ThreadCreate();
  error = OS_ThreadCreate(InitThread, // Highest priority
                          NULL,
                          &InitThreadStack[THREAD_STACK_SIZE - 1],
                          0);

  error = OS_ThreadCreate(PITThread, // 4th highest priority
                          NULL,
                          &PITThreadStack[THREAD_STACK_SIZE - 1],
                          3);

  error = OS_ThreadCreate(RTCThread, // 5th highest priority
                          NULL,
                          &RTCThreadStack[THREAD_STACK_SIZE - 1],
                          4);

  error = OS_ThreadCreate(AccelReadyThread, // 6th highest priority
                          NULL,
                          &AccelReadyThreadStack[THREAD_STACK_SIZE - 1],
                          5);

  error = OS_ThreadCreate(I2CReadCompleteThread, // 7th highest priority
                          NULL,
                          &I2CReadCompleteThreadStack[THREAD_STACK_SIZE - 1],
                          6);

  error = OS_ThreadCreate(PacketCheckerThread, // Lowest priority
                          NULL,
                          &PacketCheckerThreadStack[THREAD_STACK_SIZE - 1],
                          7);

  // Start multi-threading --> never returns
  OS_Start();

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
void StartPackets()
{
  Packet_Put(CMD_STARTUP, 0x00, 0x00, 0x00); //Tower startup packet
  Packet_Put(CMD_TWRVERSION,'v',1,0); //Tower version number
  Packet_Put(CMD_TWRNUMBER,1,NTowerNumber->s.Lo,NTowerNumber->s.Hi); // Tower number packet
  Packet_Put(CMD_TWRMODE,1,NTowerMode->s.Lo,NTowerNumber->s.Hi); // Tower mode packet
  Packet_Put(CMD_PROTOCOL,1,Protocol_Mode,0); // Protocol mode packet
}


/*! @brief Handle function for received packets
 */
void PacketHandler()
{
  bool values = false;

  LEDs_On(LED_BLUE); // Turn on blue LED

  FTMTimer0.ioType.inputDetection = TIMER_OUTPUT_HIGH; // Start FTM0 timer - channel 0
  FTM_StartTimer(&FTMTimer0);

  switch(Packet_Command & 0x7F)
  {
    case CMD_STARTUP:

      values = StartUp(); // Here we get the startup values
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

    case CMD_PROTOCOL:

      values = Protocol();
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



/*! @brief Thread that looks after interrupts made by the periodic interrupt timer.
 *
 *  @param pData Thread parameter.
 */
static void PITThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(PIT_Semaphore,0);

    static TAccelData lastAccelerometerValues; /*!< Array to store previous accelerometer data */
    uint8_t axisCount; /*!< Variables to store axis number */

    if (Protocol_Mode == ACCEL_POLL) // Only read accelerometer if in polling mode
    {
      Accel_ReadXYZ(accelerometerValues.bytes); // Collect accelerometer data
      LEDs_Toggle(LED_GREEN); // Toggle green LED

      // Send accelerometer data every second only if there is a difference from last time
      if ((lastAccelerometerValues.bytes[0] != accelerometerValues.bytes[0]) ||
         (lastAccelerometerValues.bytes[1] != accelerometerValues.bytes[1]) ||
         (lastAccelerometerValues.bytes[2] != accelerometerValues.bytes[2]))
	 {
	   Packet_Put(CMD_ACCELVALUES,accelerometerValues.bytes[0],accelerometerValues.bytes[1],accelerometerValues.bytes[2]);
         }

      for (axisCount=0; axisCount < 3; axisCount++) // Transfer data from new data array to old data array
      {
        lastAccelerometerValues.bytes[axisCount] = accelerometerValues.bytes[axisCount];
      }
    }
  }
}

/*! @brief Thread that looks after interrupts made by the real time clock.
 *
 *  @param pData Thread parameter.
 */
static void RTCThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(Update_Clock_Semaphore,0);

    uint8_t hours, minutes, seconds; /*!< Variables to store current time */

    RTC_Get(&hours,&minutes,&seconds); // Get current time each second
    Packet_Put(CMD_TIME,hours,minutes,seconds); // Update time in PC
    LEDs_Toggle(LED_YELLOW); // Toggle yellow LED
  }
}


/*! @brief User callback function for FTM0
 *
 *  @param aFTMChannel Structure containing the parameters used for setting up the timer channel
 *  @note Assumes FTM0 interrupt has occurred
 */
void FTM0Callback(const TFTMChannel* const aFTMChannel)
{
   FTMTimer0.ioType.inputDetection = TIMER_OUTPUT_DISCONNECT; // Stop timer interrupts
   FTM_StartTimer(&FTMTimer0); // Update timer setting
   LEDs_Off(LED_BLUE); // Turn off blue LED
}


/*! @brief Thread that looks after interrupts made by I2C when slave device data read is complete.
 *
 *  @param pData Thread parameter.
 */
static void I2CReadCompleteThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(Read_Complete_Semaphore,0);

    // Send accelerometer data at 1.56Hz
    Packet_Put(CMD_ACCELVALUES,accelerometerValues.bytes[0],accelerometerValues.bytes[1],accelerometerValues.bytes[2]);
  }
}


/*! @brief Thread that looks after interrupts made by the accelerometer when new data is ready.
 *
 *  @param pData Thread parameter.
 */
static void AccelReadyThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(Data_Ready_Semaphore,0);

    Accel_ReadXYZ(accelerometerValues.bytes); // Collect accelerometer data
    LEDs_Toggle(LED_GREEN); // Turn on green LED
  }
}


/*! @brief Function for obtaining the startup values
 *
 */
bool StartUp()
{
  bool success;
  success = Packet_Put(CMD_STARTUP,0x00,0x00,0x00) && Packet_Put(CMD_TWRVERSION,'v',1,0) && Packet_Put(CMD_TWRNUMBER,0x01,NTowerNumber->s.Lo,NTowerNumber->s.Hi) && Packet_Put(CMD_TWRMODE,0x01,NTowerMode->s.Lo,NTowerMode->s.Hi) && Packet_Put(CMD_PROTOCOL,1,Protocol_Mode,0); // Protocol mode;
  return success;
}


/*! @brief Function for writing a byte in memory
 *
 */
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

/*! @brief Function for reading a byte from memory
 *
 */
bool ReadByte(void)
{
  bool success;

  success = Packet_Put(0x08, Packet_Parameter1, 0, _FB(FLASH_DATA_START+Packet_Parameter1));
  return success;
}

/*! @brief Function for setting the tower version
 *
 */
bool Version(void)
{
  bool success;
  success = Packet_Put(0x09,'v',1,0);
  return success;
}

/*! @brief Function for setting the tower number
 *
 */
bool Number(void)
{

  bool success;

  if (Packet_Parameter1 == 0x01)
  { // Get tower number
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

/*! @brief Function for setting time in RTC
 *
 */
bool Time(void)
{
  bool success;

  RTC_Set(Packet_Parameter1,Packet_Parameter2,Packet_Parameter3);
  success = true;
  return success;
}

/*! @brief Function for getting or setting the tower mode
 *
 */
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

/*! @brief Function for selecting the protocol mode of the accelerator
 *
 */
bool Protocol(void)
{
  bool success;

  if (Packet_Parameter1 == 1) // Selection to get protocol mode
  {
    success = Packet_Put(CMD_PROTOCOL,1,Protocol_Mode,0); // Protocol mode
    return success;
  }

  else if (Packet_Parameter1 == 2) // Selection to set protocol mode
  {
    if (Packet_Parameter2 == 0) // Selection for asynchronous mode
    {
      Protocol_Mode = ACCEL_POLL;
      success = Packet_Put(CMD_PROTOCOL,1,Protocol_Mode,0); // Protocol mode
      Accel_SetMode(ACCEL_POLL); // Set accelerometer for polling method
      return success;
    }
    else if (Packet_Parameter2 == 1) // Selection for synchronous mode
    {
      Protocol_Mode = ACCEL_INT;
      success = Packet_Put(CMD_PROTOCOL,1,Protocol_Mode,0); // Protocol mode
      Accel_SetMode(ACCEL_INT); // Set accelerometer for interrupt method
      return success;
    }
  }
}

static void InitThread(void* pData)
{

  OS_DisableInterrupts(); // Disable interrupts

  /*!< 1 second timer on FTM0 channel 0 setup */
  FTMTimer0.channelNb = 0;
  FTMTimer0.delayCount =  CPU_MCGFF_CLK_HZ_CONFIG_0;
  FTMTimer0.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;
  FTMTimer0.ioType.outputAction = TIMER_OUTPUT_HIGH;
  FTMTimer0.userArguments =  &FTMTimer0;
  FTMTimer0.userFunction = (void* )&FTM0Callback;

  LEDs_Init(); // Initialize LED ports

  if(Packet_Init(BAUDRATE, CPU_BUS_CLK_HZ) && Flash_Init()) // UART and flash initialization
  {
    LEDs_On(LED_ORANGE); // Turn on Orange LED
  }

  if(Flash_AllocateVar((void* )&NTowerNumber, sizeof(*NTowerNumber)))
  {
    if(NTowerNumber->l == 0xFFFF)
    {
      Flash_Write16((uint16_t* )NTowerNumber, TowerNumber);
    }
  }

  if(Flash_AllocateVar((void* )&NTowerMode, sizeof(*NTowerMode)))
  {
    Flash_Write16((uint16_t* )NTowerMode, TowerMode);
  }

  RTC_Init(); // Initialize RTC
  RTC_Set(0, 0, 0); // Initial values of RTC

  PIT_Init(CPU_BUS_CLK_HZ); // Initialize PIT0
  PIT_Set(500000000*2,true); // Set PIT0 to a period of 1 second

  FTM_Init(); // Initialize FTM
  FTM_Set(&FTMTimer0); // Setup FTM0 timer --> channel 0

  Accel_Init(); // Initialize accelerometer
  Accel_SetMode(ACCEL_POLL); // Set initial mode on accelerometer

  OS_EnableInterrupts(); // Enable interrupts

  StartPackets(); // Startup packets


  OS_ThreadDelete(OS_PRIORITY_SELF); // Thread not accessed again

  return;
}

static void PacketCheckerThread(void* pData)
{
  for (;;)
  {
    if(Packet_Get()) // Check for received packets from PC
    {
      PacketHandler(); // Handle received packet
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
