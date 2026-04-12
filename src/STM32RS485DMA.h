// SPDX-License-Identifier: MIT

#pragma once
#include <Arduino.h>
#include "board_config/board_config.h"


#if (__CORTEX_M == 7)
#define RS485DMA_HAVE_DCACHE
#endif

#if defined(DMAMUX1)
#define RS485DMA_HAVE_DMAMUX
#endif


#ifdef RS485DMA_HAVE_DCACHE
#define RS485_DMA_DCACHE_CLEAN(addr, size) SCB_CleanDCache_by_Addr((uint32_t*)(addr), size)
#define RS485_DMA_DCACHE_INVALIDATE(addr, size) SCB_InvalidateDCache_by_Addr((uint32_t*)(addr), size)
#else
#define RS485_DMA_DCACHE_CLEAN(addr, size)
#define RS485_DMA_DCACHE_INVALIDATE(addr, size)
#endif

#if !defined(STM32F2xx) && \
    !defined(STM32F4xx) && \
    !defined(STM32F7xx) && \
    !defined(STM32H7xx) && \
    !defined(STM32H747xx)
#error "RS485DMA: Unsupported STM32 family (requires DMA streams: F2/F4/F7/H7)"
#endif


//Serial pins definition for each Serial available, could include RE and DE or they could be set on constructor
#if defined(ARDUINO_OPTA)
//TO DO define default config for default object
#define RS485_OPTA_DEFAULT_PINS PB_14, PB_13
#elif defined(ARDUINO_GIGA)
// TODO: No known default DE/RE pins for GIGA R1.
#elif defined(ARDUINO_PORTENTA_H7)
// TODO: No known default DE/RE pins for Portenta H7.
// Please contribute if you have tested hardware.
#ifdef(ARDUINO_PORTENTA_MACHINE_CONTROL) //macro does not exsist on arduino, user must define it
#define MACHINE_CONTROL_DEFAULT_TXRX_PINS PA_0, PI_9
#define MACHINE_CONTROL_DEFAULT_DERE_PINS PI_13, PI_10
#endif
#endif

#ifdef UART_CLEAR_TCF
#define RS485DMA_CLEAR_TCF_FLAGS(h) __HAL_UART_CLEAR_FLAG(h, UART_CLEAR_TCF)
#else
#define RS485DMA_CLEAR_TCF_FLAGS(h)
#endif 

#ifdef UART_CLEAR_TCF
#define RS485DMA_CLEAR_TCF_FLAGS(h) __HAL_UART_CLEAR_FLAG(h, UART_CLEAR_TCF)
#else
#define RS485DMA_CLEAR_TCF_FLAGS(h)
#endif

#ifdef UART_CLEAR_RTOF
#define RS485DMA_CLEAR_RTOF_FLAGS(h) __HAL_UART_CLEAR_FLAG(h, UART_CLEAR_RTOF)
#else
#define RS485DMA_CLEAR_RTOF_FLAGS(h)
#endif

#ifdef UART_CLEAR_PEF
#define RS485DMA_CLEAR_PEF_FLAGS(h) __HAL_UART_CLEAR_FLAG(h, UART_CLEAR_PEF)
#else
#define RS485DMA_CLEAR_PEF_FLAGS(h)
#endif

#ifdef UART_CLEAR_TXFECF
#define RS485DMA_CLEAR_TXFECF_FLAGS(h) __HAL_UART_CLEAR_FLAG(h, UART_CLEAR_TXFECF)
#else
#define RS485DMA_CLEAR_TXFECF_FLAGS(h)
#endif

#define MAX_RS485_INSTANCES 8

#define RS485_DEFAULT_PREDELAY 50 // us
#define RS485_DEFAULT_POSTDELAY 50  // µs
#define RS485DMA_RXSTOP_GUARD_US 2000
#define RS485DMA_TX_GUARD_US 2000

constexpr float DEFAULT_CHAR_TIME = 3.5f;


class RS485DMAClass : public Stream {
  public:
  RS485DMAClass(const RS485DMA_config* config, int, int) = delete;
  RS485DMAClass(const RS485DMA_config* config) = delete;
  RS485DMAClass(const RS485DMA_config* config, PinName dePin, PinName rePin);

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
  int readFrame(uint8_t* buffer, size_t bufferSize);
  int readBytes(uint8_t* buffer, size_t bufferSize) {
      return readFrame(buffer, bufferSize);
  };
 
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
  void checkIrqHandlers() const;

  //ISR handlers
  void usartIrqHandler();
  void txStreamIrqHandler();
  static void RxEventCallback(UART_HandleTypeDef *huart, uint16_t size);
  static void TxCpltCallback(UART_HandleTypeDef *huart);

  private:
  const RS485DMA_config* _config;

  UART_HandleTypeDef _huart;
  DMA_HandleTypeDef _hdma_rx;
  DMA_HandleTypeDef _hdma_tx;

  uint32_t _rxIdleTime = 0;
  volatile uint32_t _lastRxTimeStamp = 0;

  unsigned long _preDelay = 0;
  unsigned long _postDelay = 0;

  //PinName _txPin;
  PinName _dePin;
  PinName _rePin;
  
  volatile uint8_t _rxHead = 0;
  uint8_t _rxTail = 0;
  bool _txBusy = false;
  bool _begun = false;
  struct {
    uint32_t idleTimeStamp;
    uint8_t len = 0;
    uint8_t head = 0;
    bool armed = false;
    bool overflow = false;
  } volatile _frame;

  static RS485DMAClass* InstanceList[MAX_RS485_INSTANCES];

  static constexpr size_t DMA_RX_BUFFER_SIZE = 256;
  static constexpr size_t DMA_TX_BUFFER_SIZE = 256;

  // Keep DMA buffers at end, isolated and aligned
  alignas(32) volatile uint8_t _dma_rx_buffer[DMA_RX_BUFFER_SIZE];
  alignas(32) volatile uint8_t _dma_tx_buffer[DMA_TX_BUFFER_SIZE];
  
  bool initDMA(uint16_t config);
  void setupUsart(uint32_t baudrate);

  bool DMATxTimeOut();
  void startNextTxChunk(size_t size);
  void invalidateRxCache(size_t offset, size_t length);
  void cleanTxDCache(size_t len);

  inline size_t dma_rx_head() const { 
    uint16_t remaining = __HAL_DMA_GET_COUNTER(&_hdma_rx);
    size_t head = DMA_RX_BUFFER_SIZE - remaining;
    if (head == DMA_RX_BUFFER_SIZE) head = 0;  // explicit wrap
    return head;
  }

  static RS485DMAClass* getInstanceForUart(UART_HandleTypeDef* uart);
  void onRxIdleIRQ();
  void onTxComplete();

};


// Optional global instance for STM32DMAModbus compatibility
extern RS485DMAClass RS485;

