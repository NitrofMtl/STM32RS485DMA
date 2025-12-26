/*
  RS-485 Sender

  This example demonstrates basic RS485 transmit
  using the STM32RS485DMA library.

  On Arduino Opta, the RS485 interface is provided by a
  preconfigured hardware instance using the built-in RS485 port.

  On other STM32-based boards, an RS485DMAClass instance must
  be constructed with the appropriate Serial interface and control pins.

  This example focuses on the RS485 API usage and is independent
  of board-specific pin assignments.

  created 22 December 2025
  by NitrofMtl
*/


#include <STM32RS485DMA.h>

#define BAUDRATE 9600

int counter = 0;


void setup() {
  RS485.begin(BAUDRATE);
  RS485.noReceive();
}

void loop() {
  RS485.beginTransmission();
  RS485.print("hello ");
  RS485.println(counter);
  RS485.endTransmission();

  counter++;

  delay(1000);
}