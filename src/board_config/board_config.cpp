#include "board_config.h"

//setup GPIO
constexpr GPIO_InitTypeDef DMA_GPIO_Init(uint32_t rxPin, uint32_t txPin, uint32_t alternate)
{
    GPIO_InitTypeDef gi = {0};
    gi.Pin = rxPin | txPin;
    gi.Mode = GPIO_MODE_AF_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gi.Alternate = alternate;
    return gi;
}


constexpr RS485DMA_config RS485board_config[] =
{
#if defined(ARDUINO_OPTA)
    {
        .serial = &Serial2,
        .instance = USART3,
        .gpio_port = GPIOB,
        .usart_irqn = USART3_IRQn,
        .rx = {
            .stream = DMA1_Stream0,
            .request = DMA_REQUEST_USART3_RX,
        },
        .tx = {
            .stream = DMA1_Stream1,
            .request = DMA_REQUEST_USART3_TX,
            .irqn = DMA1_Stream1_IRQn
        },
        .GPIO = DMA_GPIO_Init(
            GPIO_PIN_10, GPIO_PIN_11,
            GPIO_AF7_USART3
        )
    },
#endif
};


const RS485DMA_config* getRS485DMAConfig(HardwareSerial& serial) {
    for (const RS485DMA_config& config : RS485board_config) {
        if (&serial == config.serial) return &config;
    }
    return nullptr; // unknown serial
}





/* //kepp for legacy for mayby stm32 that have might have fixed DMAMUX channels.. future test...
int getDMAMUXChannelIndex(DMA_Stream_TypeDef* stream) {
    if (stream >= DMA1_Stream0 && stream <= DMA1_Stream7) {
        return (stream - DMA1_Stream0); // 0..7
    } else if (stream >= DMA2_Stream0 && stream <= DMA2_Stream7) {
        return 8 + (stream - DMA2_Stream0); // 8..15
    } else {
        return -1; // invalid
    }
}
*/





/* //Old structure design, kept for port listing... nothing validated dow
constexpr DMA_Config board_config[] =
{
    // -------------------- Arduino Opta --------------------
    #if defined(ARDUINO_OPTA)
    // Serial2 → USART3
    {
        &Serial2,
        USART3,
        GPIOB,
        DMA1_Stream0,              // RX stream
        DMA_REQUEST_USART3_RX,     // RX request
        DMA1_Stream1,              // TX stream (probable, but verify)
        DMA_REQUEST_USART3_TX,      // TX request
        USART3_IRQn,
        //DMA1_Stream0_IRQn,//////////not used...
        DMA1_Stream1_IRQn,          //Tx IRQ
        DMA_GPIO_Init(
            GPIO_PIN_10 | GPIO_PIN_11,
            GPIO_AF7_USART3
        )
    },
    #endif

    // -------------------- Portenta H7 --------------------
    #if defined(ARDUINO_PORTENTA_H7_VALIDATE_CONFIG)
    // Serial1 → USART1
    {
        &Serial1,
        USART1,
        DMA2_Stream2,              // RX stream (seen in examples, verify)
        DMA_REQUEST_USART1_RX,
        DMA2_Stream3,              // TX stream (seen in examples, verify)
        DMA_REQUEST_USART1_TX
    },

    // Serial2 → USART3
    {
        &Serial2,
        USART3,
        DMA1_Stream0,              // RX stream (common mapping, verify)
        DMA_REQUEST_USART3_RX,
        DMA1_Stream1,              // TX stream (common mapping, verify)
        DMA_REQUEST_USART3_TX
    },
    #endif

    // -------------------- GIGA R1 M7 --------------------
    #if defined(ARDUINO_GIGA_VALIDATE_CONFIG)
    // Serial1 → USART1
    {
        &Serial1,
        USART1,
        DMA2_Stream2,              // RX stream (as with Portenta, verify)
        DMA_REQUEST_USART1_RX,
        DMA2_Stream3,              // TX stream (as with Portenta, verify)
        DMA_REQUEST_USART1_TX
    },

    // Serial8 → USART6
    {
        &Serial8,
        USART6,
        DMA2_Stream1,              // RX stream (probable, verify)
        DMA_REQUEST_USART6_RX,
        DMA2_Stream6,              // TX stream (probable, verify)
        DMA_REQUEST_USART6_TX
    },
    #endif
};
*/

