/*
 * packet.c
 *
 *  Created on: 28 Mar 2018
 *      Author: 13181680
 */

#include "packet.h"
#include "UART.h"

uint8_t PacketChecksum;
uint8_t state = 0;

bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk){

  return UART_Init(baudRate, moduleClk);

}

bool Packet_Get(){

  PacketChecksum = 0; // It sets the value of the checksum to zero (if value stored -> reset)

  //Start an infinite loop

  for(;;){

      switch (state){ // Selection depending on current state of the machine

	case 0:

	  if (UART_InChar(&Packet_Command)){ //Get command byte

	      state = 1; //Move to next state

	  }

	  else{

	      return false;

	  }

	  break;

	case 1:

	  if (UART_InChar(&Packet_Parameter1)){ //Get first parameter of the packet

	      state = 2; // Move to next state

	  }

	  else{
	      return false;
	  }

	  break;

	case 2:

	  if (UART_InChar(&Packet_Parameter2)){ //Get second parameter of the packet

	      state = 3; // Move to next state

	  }

	  else{
	      return false;
	  }

	  break;

	case 3:

	  if (UART_InChar(&Packet_Parameter3)){ //Get third parameter of the packet

	      state = 4; // Move to next state

	  }

	  else{
	      return false;
	  }

	  break;

	case 4:

	  if (UART_InChar(&PacketChecksum)){ //Get checksum

	      state = 5; // Move to next state

	  }

	  else{
	      return false;
	  }

	  break;

	case 5:

	  if ((Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3) == PacketChecksum)
	    {
	      state = 0; // Packet is valid, return to initial state
	      PacketChecksum = 0; // Set checksum equal zero again
	      return true;
	    }
	  else
	    {
	      Packet_Command = Packet_Parameter1; // Operations are not equal to checksum -> Right shift bytes
	      Packet_Parameter1 = Packet_Parameter2;
	      Packet_Parameter2 = Packet_Parameter3;
	      Packet_Parameter3 = PacketChecksum;
	      state = 4; // Go back to previous state and check again
	    }
	  break;

      }

  }

}


