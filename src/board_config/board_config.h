#pragma once
#include <Arduino.h>

#if defined(DMAMUX1) || defined(DMAMUX1_Channel0)
#define RS485DMA_HAVE_DMAMUX
#endif
struct SerialPinMap
{
    HardwareSerial* serial;
    PinName txPin;
    PinName rxPin;
};

struct UART_DMA_Map
{
    USART_TypeDef* instance;
    IRQn_Type irqn;
#if defined(RS485DMA_HAVE_DMAMUX)
    uint32_t dma_rx_request;
    uint32_t dma_tx_request;
#else
    uint32_t rxChannel;
    uint32_t txChannel;
#endif
};

/*struct UART_DMA_Map
{
    USART_TypeDef* instance;
    IRQn_Type irqn;
    uint32_t dma_rx_request;
    uint32_t dma_tx_request;
};*/


struct DMA_Stream_IRQ_Map
{
    DMA_Stream_TypeDef* stream;
    IRQn_Type irqn;
};


struct RS485DMA_config
{
    PinName txPin;
    PinName rxPin;
    DMA_Stream_TypeDef* rxStream;
    DMA_Stream_TypeDef* txStream;

    static const RS485DMA_config* fromPins(PinName tx, PinName rx);
    static const RS485DMA_config* fromPins(uint32_t txPin, uint32_t rxPin);
    static const RS485DMA_config* fromSerial(const HardwareSerial* serial);
    static RS485DMA_config fromPins(PinName tx, PinName rx, DMA_Stream_TypeDef* rxStream, DMA_Stream_TypeDef* txStream);
    GPIO_InitTypeDef uart_gpio() const;
    USART_TypeDef* getUsartInstance() const;
    const UART_DMA_Map* find_uart_map() const;
    const IRQn_Type txStream_irq() const;
};


// Common API
//extern const RS485DMA_config* getRS485DMAConfig(HardwareSerial& serial);
extern const RS485DMA_config* getRS485DMAConfig(USART_TypeDef* usart);

// ======================================================
// UART CLOCK ENABLE
// ======================================================
static inline void RS485DMA_EnableUARTClock(USART_TypeDef *instance)
{
    if (instance == USART1) __HAL_RCC_USART1_CLK_ENABLE();
#if defined(USART2)
    else if (instance == USART2) __HAL_RCC_USART2_CLK_ENABLE();
#endif
#if defined(USART3)
    else if (instance == USART3) __HAL_RCC_USART3_CLK_ENABLE();
#endif
#if defined(UART4)
    else if (instance == UART4)  __HAL_RCC_UART4_CLK_ENABLE();
#endif
#if defined(UART5)
    else if (instance == UART5)  __HAL_RCC_UART5_CLK_ENABLE();
#endif
}


// ======================================================
// DMA CLOCK ENABLE
// ======================================================
static inline void RS485DMA_EnableDMAClock(void)
{
#if defined(DMA1)
    __HAL_RCC_DMA1_CLK_ENABLE();
#endif
#if defined(DMA2)
    __HAL_RCC_DMA2_CLK_ENABLE();
#endif
#if defined(BDMA)
    __HAL_RCC_BDMA_CLK_ENABLE();
#endif
}


// ======================================================
// GPIO CLOCK ENABLE
// ======================================================
static inline void RS485DMA_EnableGPIOClock(GPIO_TypeDef *port)
{
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (port == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (port == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
}

inline GPIO_TypeDef* pinNameToPort(PinName pin)
{
    return reinterpret_cast<GPIO_TypeDef*>(GPIOA_BASE + (0x400 * (STM_PORT(pin))));
}

inline uint16_t pinToMask(PinName pin)
{
    return static_cast<uint16_t>(1U << STM_PIN(pin));
}