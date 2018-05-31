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

  uint8_t brfa;

  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK; // Clock gate for UART2
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // Port E enabled

  PORTE_PCR16 = PORT_PCR_MUX(3); // Port E bit 16
  PORTE_PCR17 = PORT_PCR_MUX(3); // Port E bit 17

  UART2_C1 = 0x00; // Configuration for UART
  UART2_C2 |= UART_C2_TE_MASK; // Transmission enabled
  UART2_C2 |= UART_C2_RE_MASK; // Receiving enabled (0x0C)

  UART2_C4 = UART_C4_BRFA(((moduleClk*2)/baudRate)%32);

  sbr = ((moduleClk*2)/baudRate)/32; // Calculating sbr value

  UART2_BDH |= 0x1F & (sbr >>8); // 5 most significant bits written
  UART2_BDL = sbr; // 8 least significant bits written

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
  return FIFO_Put(&TxFIFO, data); // Puts byte in transmit buffer
}


/*! @brief Poll the UART status register to try and receive and/or transmit one character.
 *
 *  @return void
 */
void UART_Poll(void)
{
  if (UART2_S1 & UART_S1_RDRF_MASK)
  {
      FIFO_Put(&RxFIFO, UART2_D);
  }
  if (UART2_S1 & UART_S1_TDRE_MASK)
  {
      FIFO_Get(&TxFIFO, (uint8_t *) &UART2_D);
  }
}
