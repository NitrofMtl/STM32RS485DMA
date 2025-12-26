// SPDX-License-Identifier: MIT

#pragma once
#include <Arduino.h>
#include "board_config/board_config.h"

#ifndef ARDUINO_ARCH_MBED
#error "RS485DMAClass: This library only supports STM32-based boards (e.g. Opta, Portenta)"
#endif

#if !(defined(ARDUINO_OPTA) || defined(ARDUINO_PORTENTA_H7) || defined(ARDUINO_GIGA))
// Add more boards here when tested
#error "RS485DMAClass: Unsupported STM32 board (only Opta, Portenta H7, Giga)"
#endif


//Serial pins definition for each Serial available, could include RE and DE or they could be set on constructor
#if defined(ARDUINO_OPTA)
#define RS485_OPTA_DEFAULT_PINS Serial2, SERIAL2_TX, PB_14, PB_13
#elif defined(ARDUINO_GIGA)
// TODO: No known default DE/RE pins for GIGA R1.
#elif defined(ARDUINO_PORTENTA_H7)
// TODO: No known default DE/RE pins for Portenta H7.
// Please contribute if you have tested hardware.
#endif

#define RS485_DEFAULT_PREDELAY 50 // us
#define RS485_DEFAULT_POSTDELAY 50  // µs
#define RS485DMA_RXSTOP_GUARD_US 2000
#define RS485DMA_TX_GUARD_US 2000

constexpr float DEFAULT_CHAR_TIME = 3.5f;


class RS485DMAClass : public Stream {
  public:
  RS485DMAClass(HardwareSerial& serial, int , int, int) = delete;
  RS485DMAClass(HardwareSerial& serial, PinName txPin, PinName dePin, PinName rePin);
  bool begin(unsigned long baudrate, uint16_t config, int predelay, int postdelay);

  // Convenience overloads
  inline bool begin(unsigned long baudrate) {
      return begin(baudrate, SERIAL_8N1,
                  RS485_DEFAULT_PREDELAY,
                  RS485_DEFAULT_POSTDELAY);
  }

  inline bool begin(unsigned long baudrate, uint16_t config) {
      return begin(baudrate, config,
                  RS485_DEFAULT_PREDELAY,
                  RS485_DEFAULT_POSTDELAY);
  }

  inline bool begin(unsigned long baudrate, int predelay, int postdelay) {
      return begin(baudrate, SERIAL_8N1, predelay, postdelay);
  }

  void end();
  int available() override;
  int peek()override;
  int read() override;
 
  void flush() override;
  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  using Print::write; // pull in write(str) and write(buf, size) from Print
  operator bool();

  void beginTransmission();
  void endTransmission();
  void receive();
  void noReceive();

  void sendBreak(uint32_t duration);
  void sendBreakMicroseconds(uint32_t duration);

  void setPins(int txPin, PinName dePin, PinName rePin);

  void setDelays(uint32_t predelay, uint32_t postdelay);

  void setRxIdleTime(uint32_t durationy);
  bool isRxIdle();
  float getBitsPerChar();
  uint32_t getUsecForNChar(float n);
  void setConfig(const RS485DMA_config* cfg);
  bool hasValidConfig() const;

  //ISR handlers
  void usartIrqHandler();
  void txStreamIrqHandler();

  private:
  const RS485DMA_config* _config;
  PinName _txPin;
  PinName _dePin;
  PinName _rePin;
  unsigned long _preDelay = 0;
  unsigned long _postDelay = 0;
  uint8_t _rxTail = 0;
  bool _txBusy = false;
  bool _begun = false;
  volatile bool _rxIdle = true;

  uint32_t _rxIdleTime = 0;
  volatile uint32_t _lastRxTimeStamp = 0;

  static constexpr size_t DMA_RX_BUFFER_SIZE = 256;
  static constexpr size_t DMA_TX_BUFFER_SIZE = 256;

  UART_HandleTypeDef _huart;
  DMA_HandleTypeDef _hdma_rx;
  DMA_HandleTypeDef _hdma_tx;

  // Keep DMA buffers at end, isolated and aligned
  alignas(32) volatile uint8_t _dma_rx_buffer[DMA_RX_BUFFER_SIZE];
  alignas(32) volatile uint8_t _dma_tx_buffer[DMA_TX_BUFFER_SIZE];
  
  bool initDMA(uint16_t config);
  void setupUsart(uint32_t baudrate);

  bool DMATxTimeOut();
  void startNextTxChunk(size_t size);
  void cleanTxDCache(size_t len);

  void onRxIdleIRQ();
  void onRxActivity();
  void onTxComplete();

  inline size_t dma_rx_head() const { 
    uint16_t remaining = __HAL_DMA_GET_COUNTER(&_hdma_rx);
    size_t head = DMA_RX_BUFFER_SIZE - remaining;
    if (head == DMA_RX_BUFFER_SIZE) head = 0;  // explicit wrap
    return head;
  }

};



#ifdef ARDUINO_OPTA
extern RS485DMAClass RS485;
#endif