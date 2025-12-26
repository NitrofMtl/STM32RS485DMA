/*
  RS-485 Sender

  This example demonstrates basic RS485 transmit and receive
  using the STM32RS485DMA library.

  This sketch relays data sent and received between the Serial port and the RS-485 interface

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
  RS485.begin(BAUDRATE);

  // enable transmission, can be disabled with: RS485.endTransmission();
  RS485.beginTransmission();

  // enable reception, can be disabled with: RS485.noReceive();
  RS485.receive();
}


// Note: RS485 is half-duplex.
// With DMA, transmit and receive are handled sequentially
// to avoid collisions and buffer corruption.
void loop() {
  // Forward Serial -> RS485
  if (Serial.available()) {
    RS485.noReceive();
    RS485.beginTransmission();
    while (Serial.available()) {
      RS485.write(Serial.read());
    }
    RS485.endTransmission();
    RS485.receive();
  }

  // Forward RS485 -> Serial
  if (RS485.available()) {
    while (RS485.available()) {
      Serial.write(RS485.read());
    }
  }
}

