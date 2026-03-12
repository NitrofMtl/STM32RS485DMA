#include "board_config.h"


static const SerialPinMap serial_pin_map[] =
{
#if SERIAL_HOWMANY > 0
    {
        &Serial1, SERIAL1_TX, SERIAL1_RX,
    },
#endif
#if SERIAL_HOWMANY > 1
    {
        &Serial2, SERIAL2_TX, SERIAL2_RX,
    },
#endif
#if SERIAL_HOWMANY > 2
    {
        &Serial3, SERIAL3_TX, SERIAL3_RX,
    },
#endif
#if SERIAL_HOWMANY > 3
    {
        &Serial4, SERIAL4_TX, SERIAL4_RX,
    },
#endif
#if SERIAL_HOWMANY > 4
    {
        &Serial5, SERIAL5_TX, SERIAL5_RX,
    },
#endif
#if SERIAL_HOWMANY > 5
    {
        &Serial6, SERIAL6_TX, SERIAL6_RX,
    },
#endif
#if SERIAL_HOWMANY > 6
    {
        &Serial7, SERIAL7_TX, SERIAL7_RX,
    },
#endif
#if SERIAL_HOWMANY > 7
    {
        &Serial8, SERIAL8_TX, SERIAL8_RX,
    },
#endif
#if SERIAL_HOWMANY > 8
    {
        &Serial9, SERIAL9_TX, SERIAL9_RX,
    },
#endif
#if SERIAL_HOWMANY > 9
    {
        &Serial10, SERIAL10_TX, SERIAL10_RX,
    },
#endif
};


static const UART_DMA_Map uart_dma_map[] =
{
#ifdef USART1
    {USART1, USART1_IRQn, DMA_REQUEST_USART1_RX, DMA_REQUEST_USART1_TX},
#endif

#ifdef USART2
    {USART2, USART2_IRQn, DMA_REQUEST_USART2_RX, DMA_REQUEST_USART2_TX},
#endif

#ifdef USART3
    {USART3, USART3_IRQn, DMA_REQUEST_USART3_RX, DMA_REQUEST_USART3_TX},
#endif

#ifdef UART4
    {UART4, UART4_IRQn, DMA_REQUEST_UART4_RX, DMA_REQUEST_UART4_TX},
#endif

#ifdef UART5
    {UART5, UART5_IRQn, DMA_REQUEST_UART5_RX, DMA_REQUEST_UART5_TX},
#endif

#ifdef USART6
    {USART6, USART6_IRQn, DMA_REQUEST_USART6_RX, DMA_REQUEST_USART6_TX},
#endif

#ifdef UART7
    {UART7, UART7_IRQn, DMA_REQUEST_UART7_RX, DMA_REQUEST_UART7_TX},
#endif

#ifdef UART8
    {UART8, UART8_IRQn, DMA_REQUEST_UART8_RX, DMA_REQUEST_UART8_TX},
#endif

#ifdef UART9
    {UART9, UART9_IRQn, DMA_REQUEST_UART9_RX, DMA_REQUEST_UART9_TX},
#endif

#ifdef USART10
    {USART10, USART10_IRQn, DMA_REQUEST_USART10_RX, DMA_REQUEST_USART10_TX},
#endif
};


static const DMA_Stream_IRQ_Map dma_stream_irq_map[] =
{

#ifdef DMA1_Stream0
    {DMA1_Stream0, DMA1_Stream0_IRQn},
#endif
#ifdef DMA1_Stream1
    {DMA1_Stream1, DMA1_Stream1_IRQn},
#endif
#ifdef DMA1_Stream2
    {DMA1_Stream2, DMA1_Stream2_IRQn},
#endif
#ifdef DMA1_Stream3
    {DMA1_Stream3, DMA1_Stream3_IRQn},
#endif
#ifdef DMA1_Stream4
    {DMA1_Stream4, DMA1_Stream4_IRQn},
#endif
#ifdef DMA1_Stream5
    {DMA1_Stream5, DMA1_Stream5_IRQn},
#endif
#ifdef DMA1_Stream6
    {DMA1_Stream6, DMA1_Stream6_IRQn},
#endif
#ifdef DMA1_Stream7
    {DMA1_Stream7, DMA1_Stream7_IRQn},
#endif

#ifdef DMA2_Stream0
    {DMA2_Stream0, DMA2_Stream0_IRQn},
#endif
#ifdef DMA2_Stream1
    {DMA2_Stream1, DMA2_Stream1_IRQn},
#endif
#ifdef DMA2_Stream2
    {DMA2_Stream2, DMA2_Stream2_IRQn},
#endif
#ifdef DMA2_Stream3
    {DMA2_Stream3, DMA2_Stream3_IRQn},
#endif
#ifdef DMA2_Stream4
    {DMA2_Stream4, DMA2_Stream4_IRQn},
#endif
#ifdef DMA2_Stream5
    {DMA2_Stream5, DMA2_Stream5_IRQn},
#endif
#ifdef DMA2_Stream6
    {DMA2_Stream6, DMA2_Stream6_IRQn},
#endif
#ifdef DMA2_Stream7
    {DMA2_Stream7, DMA2_Stream7_IRQn},
#endif

};


constexpr RS485DMA_config RS485board_config[] =
{
#if defined(ARDUINO_OPTA)
    {
        .txPin = PB_10,
        .rxPin = PB_11,
        .rxStream = DMA1_Stream0,
        .txStream = DMA1_Stream1,
    },
#endif
};


const RS485DMA_config* getRS485DMAConfig(USART_TypeDef* usart)
{
    for (const RS485DMA_config& config : RS485board_config) {
        if (config.getUsartInstance() == usart) return &config;
    }
    return nullptr; // unknown USART
}


const RS485DMA_config* RS485DMA_config::fromPins(PinName tx, PinName rx)
{
   auto instance = pinmap_peripheral(tx, PinMap_UART_TX);
    if (instance == NC) {
        //must return and invalidate obj, rs485 class MUST check for validity...
        return nullptr;
    }
    auto instance_rx = pinmap_peripheral(rx, PinMap_UART_RX);
    if (instance != instance_rx) {
        return nullptr;
    }

    return getRS485DMAConfig(reinterpret_cast<USART_TypeDef*>(instance));
}


const RS485DMA_config* RS485DMA_config::fromPins(uint32_t txPin, uint32_t rxPin)
{
    PinName tx = digitalPinToPinName(txPin);
    PinName rx = digitalPinToPinName(rxPin);

    if (tx == NC || rx == NC) {
        return nullptr;
    }
    return fromPins(tx, rx);
}


RS485DMA_config RS485DMA_config::fromPins(PinName tx, PinName rx, DMA_Stream_TypeDef* rxStream, DMA_Stream_TypeDef* txStream)
{
    RS485DMA_config config{};

    config.txPin = tx;
    config.rxPin = rx;
    config.rxStream = rxStream;
    config.txStream = txStream;

    return config;
}


const RS485DMA_config* RS485DMA_config::fromSerial(const HardwareSerial* serial)
{
    if (serial == nullptr) return nullptr;

    for (const SerialPinMap& serialMap : serial_pin_map)
    {
        if (serialMap.serial == serial) {
            return fromPins(serialMap.txPin, serialMap.rxPin);
        }
    }
    return nullptr;
}


USART_TypeDef* RS485DMA_config::getUsartInstance() const
{
    if (txPin == NC || rxPin == NC) return nullptr;

    auto instance = pinmap_peripheral(txPin, PinMap_UART_TX);
    if (instance == NC) {
        //must return and invalidate obj, rs485 class MUST check for validity...
        return nullptr;
    }
    auto instance_rx = pinmap_peripheral(rxPin, PinMap_UART_RX);
    if (instance != instance_rx) {
        return nullptr;
    }
    return reinterpret_cast<USART_TypeDef*>(instance);
}


GPIO_InitTypeDef RS485DMA_config::uart_gpio() const
{
    GPIO_InitTypeDef gi{};
    if (txPin == NC || rxPin == NC) {
        return GPIO_InitTypeDef{}; // invalid config, return empty struct
    }

    uint16_t txMask = 1U << STM_PIN(txPin);
    uint16_t rxMask = 1U << STM_PIN(rxPin);

    gi.Pin = txMask | rxMask;

    gi.Mode = GPIO_MODE_AF_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    uint32_t function = pinmap_function(txPin, PinMap_UART_TX);
    if (function == NC) {
        return GPIO_InitTypeDef{};
    }
    gi.Alternate = STM_PIN_AFNUM(function);

    return gi;
}


const UART_DMA_Map* RS485DMA_config::find_uart_map() const
{
    for (const auto& m : uart_dma_map)
    {
        if (m.instance == getUsartInstance())
            return &m;
    }
    return nullptr;
}


const IRQn_Type RS485DMA_config::txStream_irq() const
{
    for (const auto& m : dma_stream_irq_map)
    {
        if (m.stream == txStream)
            return m.irqn;
    }

    return (IRQn_Type)0;
}

