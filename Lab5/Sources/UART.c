/*! @file UART.c
 *
 *  @brief This file contents the initialization and implementation of UART functions
 *
 *  @author 13181680  & 12099115
 *  @date 28 Mar 2018
 */

 /*!
  *  @addtogroup UART_module UART module documentation
  *  @{
  */

#include "MK70F12.h"
#include "types.h"
#include "FIFO.h"
#include "Cpu.h"
#include "PE_Types.h"
#include "OS.h"

TFIFO TxFIFO;
TFIFO RxFIFO;

// Thread stack size - big enough for stacking of interrupts and OS use.
#define THREAD_STACK_SIZE 100

// Threads
static void ReceiveThread(void* pData);
static void TransmitThread(void* pData);

static uint32_t TransmitThreadStack[THREAD_STACK_SIZE]; /*!< The stack for the transmit thread. */
static uint32_t ReceiveThreadStack[THREAD_STACK_SIZE]; /*!< The stack for the receive thread */
static OS_ECB *TransmitSemaphore; /*!< Binary semaphore for signaling that data transmission */
static OS_ECB *ReceiveSemaphore;  /*!< Binary semaphore for signaling receiving of data */


/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @return bool - TRUE if the UART was successfully initialized.
 */
bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{

  OS_ERROR error; //Thread content

  uint16_t sbr;
  uint16_t baudRateDivisor;

  uint8_t brfa;

  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK; // Clock gate for UART2
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // Port E enabled

  PORTE_PCR16 = PORT_PCR_MUX(3); // Port E bit 16
  PORTE_PCR17 = PORT_PCR_MUX(3); // Port E bit 17

  // Disable the module
  UART2_C2 &= ~UART_C2_TE_MASK; // Transmission enabled
  UART2_C2 &= ~UART_C2_RE_MASK; // Receiving enabled (0x0C)

  UART2_C1 = 0x00; // Configuration for UART
  UART2_C2 |= UART_C2_RIE_MASK; // Receiving interrupts enabled

  baudRateDivisor = (moduleClk*2)/baudRate;
  brfa = baudRateDivisor%32;

  UART2_C4 = UART_C4_BRFA(brfa);

  sbr = baudRateDivisor/32; // Calculating sbr value

  UART2_BDH |= 0x1F & (sbr >>8); // 5 most significant bits written
  UART2_BDL = sbr; // 8 least significant bits written

  // Now at the end of config we enable the module
  UART2_C2 |= UART_C2_TE_MASK; // Transmission enabled
  UART2_C2 |= UART_C2_RE_MASK; // Receiving enabled (0x0C)

  NVICICPR1 = NVIC_ICPR_CLRPEND(1 << 17); // Clear any pending interrupts on UART2
  NVICISER1 = NVIC_ISER_SETENA(1 << 17);  // Enable interrupts on UART2

  FIFO_Init(&RxFIFO); // Initialize receiver
  FIFO_Init(&TxFIFO); // Initialize transmitter

  ReceiveSemaphore = OS_SemaphoreCreate(0); // Receive semaphore initialized to 0
  TransmitSemaphore = OS_SemaphoreCreate(0); // Transmit semaphore initialized to 0

  error = OS_ThreadCreate(ReceiveThread, NULL, &ReceiveThreadStack[THREAD_STACK_SIZE - 1], 1); // 2nd highest priority thread

  error = OS_ThreadCreate(TransmitThread, NULL, &TransmitThreadStack[THREAD_STACK_SIZE - 1], 2); // 3rd highest priority thread

  return true;
}

/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return bool - TRUE if the receive FIFO returned a character.
 */
bool UART_InChar(uint8_t * const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr); // Gets oldest byte from buffer
}

/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return bool - TRUE if the data was placed in the transmit FIFO.
 */
bool UART_OutChar(const uint8_t data)
{
  EnterCritical();
  bool success = FIFO_Put(&TxFIFO, data);
  if(success)
  {
    UART2_C2 |= UART_C2_TIE_MASK;
  }

  ExitCritical();
  return success; // Puts byte in transmit buffer
}

/*! @brief Thread that looks after receiving data.
 *
 *  @param pData Thread parameter.
 *  @note Assumes that semaphores are created and communicate properly.
 */
static void ReceiveThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(ReceiveSemaphore, 0); // Wait for receive semaphore to signal
    FIFO_Put(&RxFIFO, UART2_D); // Put byte into RxFIFO
    UART2_C2 |= UART_C2_RIE_MASK; // Re-enable receive interrupt
  }
}

/*! @brief Thread that looks after transmitting data.
 *
 *  @param pData Thread parameter.
 *  @note Assumes that semaphores are created and communicate properly.
 */
void TransmitThread(void *pData)
{
  for (;;)
  {
    OS_SemaphoreWait(TransmitSemaphore, 0); // Wait for transmit semaphore to signal
    if (UART2_S1 & UART_S1_TDRE_MASK) // Clear TDRE flag by reading it
    {
      FIFO_Get(&TxFIFO,(uint8_t* )&UART2_D);
      UART2_C2 |= UART_C2_TIE_MASK; // Re-enable transmission interrupt
    }
  }
}


/*! @brief Interrupt service routine for the UART.
 *
 *  @note Assumes the transmit and receive FIFOs have been initialized.
 */
void __attribute__ ((interrupt)) UART_ISR(void)
{
  OS_ISREnter(); // Start of servicing interrupt

  if(UART2_C2 & UART_C2_TIE_MASK) // Only respond if transmit(TDRE) interrupt is enabled
  {
    if(UART2_S1 & UART_S1_TDRE_MASK) // Clear TDRE flag by reading it
    {
      UART2_C2 &= ~UART_C2_TIE_MASK; // Transmit interrupt disabled
      OS_SemaphoreSignal(TransmitSemaphore); // Signal transmit thread
    }
  }
  if(UART2_C2 & UART_C2_RIE_MASK) // Only respond if transmit(RDRF) interrupt is enabled
  {
    if(UART2_S1 & UART_S1_RDRF_MASK) // Clear RDRF flag by reading it
    {
      UART2_C2 &= ~UART_C2_RIE_MASK; // Receive interrupt disabled
      OS_SemaphoreSignal(ReceiveSemaphore); // Signal receive thread
    }
  }
  OS_ISRExit(); // Exit of servicing interrupt
}

/*!
 * @}
*/
