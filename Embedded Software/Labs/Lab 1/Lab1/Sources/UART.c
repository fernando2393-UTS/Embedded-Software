/*
 * UART.c
 *
 *  Created on: 23 Mar 2018
 *      Author: 13181680
 */

static TFIFO, TxFIFO, RxFIFO;

bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk){
  // Turn uART32 on
  SIM_SCGC4 = SIM_SCGC4_UART2_MASK;
  // Tx Pin
  PORTE_PCR16 = PORT_PCR_MUX(3);
  // Rx Pin
  PORTE_PCR17 = PORT_PCR_MUX(3);

  // Clear bit 4 which is w1c
  // Example: 00010011 --> 00000011

  REGISTER_WITH_W1C_BITS = 0b0001000;
}


