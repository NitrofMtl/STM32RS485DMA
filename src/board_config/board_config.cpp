#include "board_config.h"


#ifdef RS485DMA_HAVE_DMAMUX
static const UART_DMA_Map uart_dma_map[] =
{
#ifdef USART1
    {USART1, USART1_IRQn, DMA_REQUEST_USART1_RX, DMA_REQUEST_USART1_TX, DMA1_Stream0, DMA1_Stream1},
#endif

#ifdef USART2
    {USART2, USART2_IRQn, DMA_REQUEST_USART2_RX, DMA_REQUEST_USART2_TX, DMA1_Stream2, DMA1_Stream3},
#endif

#ifdef USART3
    {USART3, USART3_IRQn, DMA_REQUEST_USART3_RX, DMA_REQUEST_USART3_TX, DMA1_Stream4, DMA1_Stream5},
#endif

#ifdef UART4
    {UART4, UART4_IRQn, DMA_REQUEST_UART4_RX, DMA_REQUEST_UART4_TX, DMA1_Stream6, DMA1_Stream7},
#endif

#ifdef UART5
    {UART5, UART5_IRQn, DMA_REQUEST_UART5_RX, DMA_REQUEST_UART5_TX, DMA2_Stream0, DMA2_Stream1},
#endif

#ifdef USART6
    {USART6, USART6_IRQn, DMA_REQUEST_USART6_RX, DMA_REQUEST_USART6_TX, DMA2_Stream2, DMA2_Stream3},
#endif

#ifdef UART7
    {UART7, UART7_IRQn, DMA_REQUEST_UART7_RX, DMA_REQUEST_UART7_TX, DMA2_Stream4, DMA2_Stream5},
#endif

#ifdef UART8
    {UART8, UART8_IRQn, DMA_REQUEST_UART8_RX, DMA_REQUEST_UART8_TX, DMA2_Stream6, DMA2_Stream7},
#endif

#ifdef UART9
    {UART9, UART9_IRQn, DMA_REQUEST_UART9_RX, DMA_REQUEST_UART9_TX, DMA2_Stream0, DMA2_Stream1},
#endif

#ifdef USART10
    {USART10, USART10_IRQn, DMA_REQUEST_USART10_RX, DMA_REQUEST_USART10_TX, DMA2_Stream1, DMA2_Stream2},
#endif
};
#else
static const UART_DMA_Map uart_dma_map[] =
{
#ifdef USART1
    {USART1, USART1_IRQn, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA2_Stream2, DMA2_Stream7},
#endif

#ifdef USART2
    {USART2, USART2_IRQn, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA1_Stream5, DMA1_Stream6},
#endif

#ifdef USART3
    {USART3, USART3_IRQn, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA1_Stream1, DMA1_Stream3},
#endif

#ifdef UART4
    {UART4, UART4_IRQn, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA1_Stream2, DMA1_Stream4},
#endif

#ifdef UART5
    {UART5, UART5_IRQn, DMA_CHANNEL_4, DMA_CHANNEL_4, DMA1_Stream0, DMA1_Stream7},
#endif

#ifdef USART6
    {USART6, USART6_IRQn, DMA_CHANNEL_5, DMA_CHANNEL_5, DMA2_Stream1, DMA2_Stream6},
#endif

#ifdef UART7
    {UART7, UART7_IRQn, DMA_CHANNEL_5, DMA_CHANNEL_5, DMA1_Stream3, DMA1_Stream1},
#endif

#ifdef UART8
    {UART8, UART8_IRQn, DMA_CHANNEL_5, DMA_CHANNEL_5, DMA1_Stream6, DMA1_Stream0},
#endif
};
#endif


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


const RS485DMA_config RS485DMA_config::fromPins(PinName tx, PinName rx)
{
    RS485DMA_config config{};

    config.txPin = tx;
    config.rxPin = rx;

    const UART_DMA_Map* usartMap = config.find_uart_map();
    if (!usartMap) {
        // handle invalid pin combination
        config.rxStream = nullptr;
        config.txStream = nullptr;
        return config;
    }
    config.rxStream = usartMap->rx_stream;
    config.txStream = usartMap->tx_stream;

    return config;
}


const RS485DMA_config RS485DMA_config::fromPins(PinName tx, PinName rx, DMA_Stream_TypeDef* rxStream, DMA_Stream_TypeDef* txStream)
{
    RS485DMA_config config{};

    config.txPin = tx;
    config.rxPin = rx;
    config.rxStream = rxStream;
    config.txStream = txStream;

    return config;
}


USART_TypeDef* RS485DMA_config::getUsartInstance() const
{
    if (txPin == NC || rxPin == NC) return nullptr;

    USART_TypeDef* instance = static_cast<USART_TypeDef*>(pinmap_peripheral(txPin, PinMap_UART_TX));
    if (instance == reinterpret_cast<USART_TypeDef*>(NC)){
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

