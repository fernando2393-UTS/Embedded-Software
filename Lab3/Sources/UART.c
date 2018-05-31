/*! @file
 *
 * @brief This file contents the initialization and implementation of UART functions
 *
 *  Created on: 28 Mar 2018
 *      Author: 13181680
 */

#include "MK70F12.h"
#include "types.h"
#include "FIFO.h"
#include "Cpu.h"
#include "PE_Types.h"

TFIFO TxFIFO;
TFIFO RxFIFO;


/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @return bool - TRUE if the UART was successfully initialized.
 */
bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{

  uint16_t sbr;
  uint16_t baudRateDivisor;

  uint8_t brfa;

  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK; // Clock gate for UART2
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // Port E enabled

  PORTE_PCR16 = PORT_PCR_MUX(3); // Port E bit 16
  PORTE_PCR17 = PORT_PCR_MUX(3); // Port E bit 17

  // Disable the module
  UART2_C2 &= ~UART_C2_TE_MASK; // Transmission disabled
  UART2_C2 &= ~UART_C2_RE_MASK; // Receiving disabled (0x0C)

  UART2_C1 = 0x00; // Configuration for UART
//  UART2_C2 |= UART_C2_TIE_MASK; // Transmission interrupts enabled
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



//Poll won't be needed, now we use a system of interrupts


/*! @brief Interrupt service routine for the UART.
 *
 *  @note Assumes the transmit and receive FIFOs have been initialized.
 */
void __attribute__ ((interrupt)) UART_ISR(void)
{
        if(UART2_C2 & UART_C2_TIE_MASK) // Only respond if transmit(TDRE) interrupt is enabled
        {
                if(UART2_S1 & UART_S1_TDRE_MASK) // Clear TDRE flag by reading it
                {
                        if(!FIFO_Get(&TxFIFO,(uint8_t* const)&UART2_D)) // Check if transmitter FIFO is empty
                        {
                                UART2_C2 &= ~UART_C2_TIE_MASK; // Transmit(TDRE) interrupt disabled
                        }
                }
        }
        if(UART2_C2 & UART_C2_RIE_MASK) // Only respond if transmit(RDRF) interrupt is enabled
        {
                if(UART2_S1 & UART_S1_RDRF_MASK) // Clear RDRF flag by reading it
                {
                        FIFO_Put(&RxFIFO, UART2_D); // Place data in receiver FIFO

                }
        }
}
