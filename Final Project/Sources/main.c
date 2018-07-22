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
#include "PIT.h"
#include "OS.h"
#include "FTM.h"
#include "analog.h"
#include "Calculations.h"
#include "analogMeasure.h"
#include "voltageRegulator.h"

// Thread stack size - big enough for stacking of interrupts and OS use.
#define THREAD_STACK_SIZE 100

static const int BAUDRATE = 115200;

// Protocol packet definitions
#define CMD_STARTUP 0x04
#define CMD_WRITEBYTE 0x07
#define CMD_READBYTE 0x08
#define CMD_TWRVERSION 0x09
#define CMD_PROTOCOL 0x0A
#define CMD_TWRNUMBER 0x0B
#define CMD_TWRMODE 0x0D
#define CMD_TIMINGMODE 0x10
#define CMD_RISES 0x11
#define CMD_LOWERS 0x12
#define CMD_FREQUENCY 0x17
#define CMD_RMSVOLTAGE 0x18
#define CMD_SPECTRUM 0x19
#define ACK_REQUEST_MASK 0x80

static void InitThread(void* pData);
static void PacketCheckerThread(void* pData);
static void PITThread(void* pData);
static void PITThread1(void* pData);
static void PITThread2(void* pData);
void StartPackets();
void PacketHandler();

bool StartUp();
bool WriteByte();
bool ReadByte();
bool Version();
bool Number();
bool Mode();
bool Protocol();
bool Raises();
bool Lowers();
bool TimingMode();
bool Frequency();
bool valuesVRMS();
bool Spectrum();

uint16union_t* NTowerNumber;
uint16union_t* NTowerMode;

static TFTMChannel FTMTimer0;          /*!< Stores content of 1 second timer on FTM0 channel 0 */

static uint16_t TowerNumber = 1680; //Last 4 digits of student ID (0x690)
static uint16_t TowerMode = 2; //Initial tower mode

TFIFO TxFIFO, RxFIFO;


TPacket Packet;

// Threads initialization

static uint32_t InitThreadStack[THREAD_STACK_SIZE];            /*!< The stack for the initialization thread. */
static uint32_t PacketCheckerThreadStack[THREAD_STACK_SIZE];   /*!< The stack for the packet checking thread. */
static uint32_t AccelReadyThreadStack[THREAD_STACK_SIZE];      /*!< The stack for the accelerometer data ready thread. */
static uint32_t I2CReadCompleteThreadStack[THREAD_STACK_SIZE]; /*!< The stack for the I2C read complete thread. */
static uint32_t PITThreadStack[THREAD_STACK_SIZE];             /*!< The stack for the PIT thread. */
static uint32_t PIT1ThreadStack[THREAD_STACK_SIZE];             /*!< The stack for the PIT thread. */
static uint32_t PIT2ThreadStack[THREAD_STACK_SIZE];             /*!< The stack for the PIT thread. */

/*static uint32_t AnalogThreadStack[THREAD_STACK_SIZE];*/

// Semaphores initialization

OS_ECB *Data_Ready_Semaphore;    /*!< Binary semaphore for signaling that data is ready to be read on accelerometer */
OS_ECB *Read_Complete_Semaphore; /*!< Binary semaphore for signaling that data was read successfully */
OS_ECB *PIT_Semaphore;           /*!< Binary semaphore for signaling PIT interrupt */
OS_ECB *PIT_Semaphore1;          /*!< Binary semaphore for signaling PIT1 interrupt */
OS_ECB *PIT_Semaphore2;          /*!< Binary semaphore for signaling PIT1 interrupt */

//Voltage samples array

channelSample samples [3];

int8_t NumberOfRaises; // Byte to store the number of highers
int8_t NumberOfLowers; // Byte to store the number of lowers

int8_t timingMode; // Byte which stores the timing mode (definite or inverse)
int8_t* NtimingMode = &timingMode; // Pointer to timingMode

int8_t* NRaises = &NumberOfRaises; // Pointer to numberOfHighers
int8_t* NLowers = &NumberOfLowers; // Pointer to NumberOfLowers

int32_t measurementsFreq; // Period for PIT0 with the current measurement frequency

float frequency; // It stores the value of the frequency

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

  error = OS_ThreadCreate(PITThread, // 3th highest priority
                          NULL,
                          &PITThreadStack[THREAD_STACK_SIZE - 1],
                          3);

  error = OS_ThreadCreate(PITThread2, // 4th highest priority
                          NULL,
                          &PIT2ThreadStack[THREAD_STACK_SIZE - 1],
                          4);

  error = OS_ThreadCreate(PITThread1, // 5th highest priority
                          NULL,
                          &PIT1ThreadStack[THREAD_STACK_SIZE - 1],
                          5);

  error = OS_ThreadCreate(PacketCheckerThread, // Lowest priority
                          NULL,
                          &PacketCheckerThreadStack[THREAD_STACK_SIZE - 1],
                          6);


  // Start multi-threading --> never returns
  OS_Start();

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
        PEX_RTOS_START();              /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
        /*** End of RTOS startup code.  ***/
        /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
        for (;;) {}
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

    case CMD_TIMINGMODE:

      values = TimingMode();
      break;

    case CMD_LOWERS:

      values = Lowers();
      break;

    case CMD_RISES:

      values = Raises();
      break;

    case CMD_FREQUENCY:

      values = Frequency();
      break;

    case CMD_RMSVOLTAGE:

      values = valuesVRMS();
      break;

    case CMD_SPECTRUM:

      values = Spectrum();
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



/*! @brief Thread that looks after interrupts made by the periodic interrupt timer 0. Uses to calculate VRMS of each channel and check them.
 *
 *  @param pData Thread parameter.
 */
static void PITThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(PIT_Semaphore,0);

    // Temporal variable which store VRMS
    static int16_t tempval;

    // VRMS calculation and storage
    tempval = VRMS(samples[0].myArray);
    samples[0].vrmsValue = tempval;
    if (tempval < 4901.55)
    {
      PIT_Set(1250000, false); // If frequency is below 1.5 V in VRMS, do not apply frequency tracking --> Default of 1.25 ms period
    }
    else
    {
      calculateMinimum();
      PIT_Set(measurementsFreq, false); // PIT is now set with new frequency
    }
    //Boundaries check
    vrmsChecker(tempval, 0);
  }
}

/*! @brief Thread that looks after interrupts made by the periodic interrupt timer 1. It decides the mode of timing.
 *
 *  @param pData Thread parameter.
 */
static void PITThread1(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(PIT_Semaphore1,0);
    if (timingMode == 1)
    {
      definiteCheck();
    }
    else if (timingMode == 2)
    {
      inverseTimeMode();
    }
  }
}

static void PITThread2(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(PIT_Semaphore2,0);

    // Temporal variables which store VRMS
    static int16_t tempval1;
    static int16_t tempval2;

    // VRMS calculation and storage for the different channels
    tempval1 = VRMS(samples[1].myArray);
    tempval2 = VRMS(samples[2].myArray);
    samples[1].vrmsValue = tempval1;
    samples[2].vrmsValue = tempval2;

    //Boundaries check
    vrmsChecker(tempval1, 1);
    vrmsChecker(tempval2, 2);
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


/*! @brief Function for obtaining the startup values
 *  @return bool success either true or false
 */
bool StartUp()
{
  bool success;
  success = Packet_Put(CMD_STARTUP,0x00,0x00,0x00) && Packet_Put(CMD_TWRVERSION,'v',1,0) && Packet_Put(CMD_TWRNUMBER,0x01,NTowerNumber->s.Lo,NTowerNumber->s.Hi) && Packet_Put(CMD_TWRMODE,0x01,NTowerMode->s.Lo,NTowerMode->s.Hi); // Protocol mode;
  return success;
}


/*! @brief Function for writing a byte in memory
 *  @return bool success either true or false
 */
bool WriteByte(void)
{

  bool success;

  if (Packet_Parameter1 > 7)
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
 *  @return bool success either true or false
 */
bool ReadByte(void)
{
  bool success;

  success = Packet_Put(0x08, Packet_Parameter1, 0, _FB(FLASH_DATA_START+Packet_Parameter1));
  return success;
}

/*! @brief Function for setting the tower version
 *  @return bool success either true or false
 */
bool Version(void)
{
  bool success;
  success = Packet_Put(0x09,'v',1,0);
  return success;
}

/*! @brief Function for setting the tower number
 *  @return bool success either true or false
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
    if (Flash_AllocateVar((void*)&NTowerNumber, sizeof(*NTowerNumber)))
    {
      success = Flash_Write16((uint16_t*) NTowerNumber, Packet_Parameter23);
      return success;
    }
  }
}


/*! @brief Function for getting or setting the tower mode
 *  @return bool success either true or false
 */
bool Mode(void)
{
  bool success;

  if (Packet_Parameter1 == 1) //Get tower mode
  {
    success = Packet_Put(0x0D, 1, NTowerMode->s.Lo, NTowerMode->s.Hi);
    return success;
  }

  else if (Packet_Parameter1 == 2)
  {
    if (Flash_AllocateVar((void*)&NTowerMode, sizeof(*NTowerMode)))
    {
      success = Flash_Write16((uint16_t*) NTowerMode, Packet_Parameter23);
      return success;
    }
  }
}

/*! @brief Function for getting or setting the timing mode
 *  @return bool success either true or false
 */
bool TimingMode(void)
{
  bool success;
  if (Packet_Parameter1 == 0) //Get tower timing mode
  {
    success = Packet_Put(0x10, timingMode, 0, 0);
  }
  else if (Packet_Parameter1 == 1 || Packet_Parameter1 == 2)
  {
//    success = Flash_Write8((int8_t*) NtimingMode, Packet_Parameter1);
    timingMode = Packet_Parameter1;
    success = true;
  }
  return success;
}

/*! @brief Function for getting or resetting the number of raises
 *  @return bool success either true or false
 */
bool Raises(void)
{
  bool success;
  if (Packet_Parameter1 == 0) // Get number of raises
  {
    success = Packet_Put(0x11, NumberOfRaises, 0, 0);
  }
  else if (Packet_Parameter1 == 1)
  {
//    success = Flash_Write8((uint8_t*)NRaises, 0);
    NumberOfRaises = 0;
    success = true;
  }
  return success;
}

/*! @brief Function for getting or resetting the number of lowers
 *  @return bool success either true or false
 */
bool Lowers(void)
{
  bool success;
  if (Packet_Parameter1 == 0) // Get number of raises
  {
    success = Packet_Put(0x12, NumberOfLowers, 0, 0);
  }
  else if (Packet_Parameter1 == 1)
  {
//    success = Flash_Write8((int8_t* )NLowers, 0);
    NumberOfLowers = 0;
    success = true;
  }
  return success;
}

/*! @brief Function for getting the value of the current frequency
 *  @return bool success either true or false
 */
bool Frequency(void)
{
  bool success;
  uint16_t tempfreq = (uint16_t) (frequency * 10);
  uint16_t highfreq = tempfreq/256;
  uint16_t lowfreq = tempfreq%256;
  uint8_t highfreq8 = (uint8_t) highfreq;
  uint8_t lowfreq8 = (uint8_t) lowfreq;
  success = Packet_Put(0x17, lowfreq8, highfreq8, 0);
  return success;
}

/*! @brief Function for getting the value of the VRMS of one channel
 *  @return bool success either true or false
 */
bool valuesVRMS(void)
{
  bool success;
  if (Packet_Parameter1 == 1)
  {
    uint16_t tempVRMS = (uint16_t) ((samples[0].vrmsValue / 3276.7) * 100);
    uint16_t lowVRMS = tempVRMS%256;
    uint16_t highVRMS = tempVRMS/256;
    uint8_t highVRMS8 = (uint8_t) highVRMS;
    uint8_t lowVRMS8 = (uint8_t) lowVRMS;
    success = Packet_Put(0x18, Packet_Parameter1, lowVRMS8, highVRMS8);
  }
  else if (Packet_Parameter1 == 2)
  {
    uint16_t tempVRMS = (uint16_t) ((samples[1].vrmsValue / 3276.7) * 100);
    uint16_t lowVRMS = tempVRMS%256;
    uint16_t highVRMS = tempVRMS/256;
    uint8_t highVRMS8 = (uint8_t) highVRMS;
    uint8_t lowVRMS8 = (uint8_t) lowVRMS;
    success = Packet_Put(0x18, Packet_Parameter1, lowVRMS8, highVRMS8);
  }
  else if (Packet_Parameter1 == 3)
  {
    uint16_t tempVRMS = (uint16_t) ((samples[2].vrmsValue / 3276.7) * 100);
    uint16_t lowVRMS = tempVRMS%256;
    uint16_t highVRMS = tempVRMS/256;
    uint8_t highVRMS8 = (uint8_t) highVRMS;
    uint8_t lowVRMS8 = (uint8_t) lowVRMS;
    success = Packet_Put(0x18, Packet_Parameter1, lowVRMS8, highVRMS8);
  }
  return success;
}

/*! @brief Function for getting the value of the spectrum of channel 1
 *  @return bool success either true or false
 */
bool Spectrum()
{
  float amp[9]; //16/2 + 1
  FFT_Freq(samples[0].myArray, amp);
  bool success;
  for (int i = 0; i < 8; i++)
  {
    uint16_t auxAmp = *(amp+i+1);
    success &= Packet_Put(0x19, i, ((uint16union_t)auxAmp).s.Lo, ((uint16union_t)auxAmp).s.Hi);
  }
  return success;
}

/*! @brief Function for initializing the components
 *  @note It destroys itself after being executed
 */
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

  if (Packet_Init(BAUDRATE, CPU_BUS_CLK_HZ) && Flash_Init()) // UART and flash initialization
  {
    LEDs_On(LED_ORANGE); // Turn on Orange LED
  }

  if (Flash_AllocateVar((void* )&NTowerNumber, sizeof(*NTowerNumber)))
  {
    if (NTowerNumber->l == 0xFFFF)
    {
      Flash_Write16((uint16_t* )NTowerNumber, TowerNumber);
    }
  }

  if (Flash_AllocateVar((void* )&NTowerMode, sizeof(*NTowerMode)))
  {
    Flash_Write16((uint16_t* )NTowerMode, TowerMode);
  }

  if (Flash_AllocateVar((void*)&NRaises, sizeof(*NRaises)))
  {
    Flash_Write8((int8_t* )NRaises, 0);
  }

  if (Flash_AllocateVar((void*)&NLowers, sizeof(*NLowers)))
  {
    Flash_Write8((int8_t* )NLowers, 0);
  }

  if (Flash_AllocateVar((void*)&NtimingMode, sizeof(*NtimingMode)))
  {
    Flash_Write8((int8_t* )NtimingMode, 1);
  }

  timingMode = 1; // Initialized to definite by default


  PIT_Init(CPU_BUS_CLK_HZ); // Initialize PIT0
  measurementsFreq = 1250000;
  PIT_Set(measurementsFreq, true); // Set period of PIT0 to 1.25 ms
  PIT_Set2(1250000, true); // Set period of PIT2 to 1.25 ms

  FTM_Init(); // Initialize FTM
  FTM_Set(&FTMTimer0); // Setup FTM0 timer --> channel 0

  (void) Analog_Init(CPU_BUS_CLK_HZ);

  OS_EnableInterrupts(); // Enable interrupts

  StartPackets(); // Startup packets


  OS_ThreadDelete(OS_PRIORITY_SELF); // Thread not accessed again

  return;
}

/*! @brief Function for checking if new packets arrive
 *  @return void
 */
static void PacketCheckerThread(void* pData)
{
  for (;;)
  {
    if (Packet_Get()) // Check for received packets from PC
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
