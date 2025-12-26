/*
  RS-485 Receiver

  This example demonstrates basic RS485 receive
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

void setup() {
  Serial.begin(9600);
  while (!Serial);

  RS485.begin(BAUDRATE);

  // enable reception, can be disabled with: RS485.noReceive();
  RS485.receive();
}

void loop() {
  if (RS485.available()) {
    Serial.write(RS485.read());
  }
}

