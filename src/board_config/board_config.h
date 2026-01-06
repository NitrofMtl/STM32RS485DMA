#pragma once
#include <Arduino.h>


constexpr GPIO_InitTypeDef DMA_GPIO_Params(uint32_t rxPin, uint32_t txPin, uint32_t alternate);


struct RS485DMA_config {
    HardwareSerial* serial;
    USART_TypeDef* instance;
    GPIO_TypeDef* gpio_port;
    IRQn_Type usart_irqn;
    struct {
        DMA_Stream_TypeDef* stream;
        uint32_t request;
    } rx;
    struct {
        DMA_Stream_TypeDef* stream;
        uint32_t request;
        IRQn_Type irqn;
    } tx;
    GPIO_InitTypeDef GPIO;
};


// Common API
extern const RS485DMA_config* getRS485DMAConfig(HardwareSerial& serial);


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

