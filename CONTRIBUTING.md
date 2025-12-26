
Thanks for your interest in contributing.

This library targets STM32 DMA-based UART peripherals and requires
a solid understanding of:
- STM32 HAL
- UART + DMA interaction
- Cache coherency (DCache)
- Interrupt handling

Basic Arduino usage questions should go to Discussions, not PRs.
---

## What we accept
- New STM32 board support
- DMA / timing fixes
- Documentation improvements
- Examples

---

## What we don't accept (without discussion)
- Blocking APIs
- Polling-based RX/TX
- Non-DMA implementations
- Changes that break Arduino `Stream` semantics

---

## Adding a new board
Please provide:
- Board name
- UART instance used
- DMA RX/TX stream or channel
- DE / RE pin mapping
- Confirmation of cache requirements

---

## Code style
- No dynamic allocation
- Explicit state machines
- Defensive HAL usage
- No hidden background tasks

---

## Library architecture overview

Each supported board provides:
- A static DMA_Config structure
- Predefined UART / DMA / GPIO mappings
- ISR forwarding functions

The core library logic is hardware-agnostic and relies on these configurations.


---

## Adding support for a new board

1. Create a board-specific configuration file
   - Define UART instance
   - Define DMA streams and requests
   - Define GPIO alternate functions

2. Declare a `DMA_Config` instance for the board

3. Provide IRQ forwarding handlers
   - USARTx_IRQHandler → RS485DMAClass::usartIrqHandler()
   - DMA RX stream IRQ → rxStreamIrqHandler()
   - DMA TX stream IRQ → txStreamIrqHandler()

4. Instantiate a global RS485 object (if applicable)

5. Validate RX (DMA + IDLE) before TX


---

## Interrupt handling

ISR symbols are defined by the MCU startup files and cannot be dynamically registered.

For this reason:
- IRQ handlers are board-specific
- Handlers must forward to the RS485DMAClass instance

Do not attempt to generate ISR names dynamically.


---

## STM32 family differences

When porting to a new MCU family, pay attention to:

- DMA vs BDMA vs GPDMA
- Presence or absence of DMAMUX
- UART FIFO support
- RCC clock tree differences
- Cache (DCache) requirements
- HAL macro availability

Some of these may require conditional compilation or HAL workarounds.


---

## Pull request guidelines

- Board-specific code should be isolated
- Do not hardcode USART instances in the core
- Avoid breaking Opta support
- Keep Arduino-style API compatibility


---
#### 1. Using built-in board configurations
No configuration required. Supported boards automatically select
the correct DMA and USART mapping.

#### 2. Providing a custom DMA configuration (recommended for new boards)
Advanced users may define a DMA_Config directly in their sketch and
attach it using setConfig() before calling begin().

---
## Example of implementation
---

````

#include "STM32RS485DMA.h"

/// -----------------------------------------------------------------------------
// DMA / USART configuration
// Fill these according to the MCU reference manual.
//
// Each RS485 port requires:
//  - one USART
//  - one TX DMA stream
//  - one RX DMA stream
//  - its own IRQ handlers
// -----------------------------------------------------------------------------
constexpr DMA_Config RS485x_Config = 
{
    .serial = &SerialX,
    .instance = USARTx,
    .gpio_port = GPIOX,
    .usart_irqn = USARTx_IRQn,
    .rx = {
        .stream = DMAx_Streamy,
        .request = DMA_REQUEST_USARTx_RX,
    },
    .tx = {
        .stream = DMAx_Streamz,
        .request = DMA_REQUEST_USARTx_TX,
        .irqn = DMAx_Streamz_IRQn
    },
    .GPIO = DMA_GPIO_Init(
        GPIO_PIN_tx, GPIO_PIN_rx,
        GPIO_AF7_USART3
    )
};

// -----------------------------------------------------------------------------
// RS485 object
// -----------------------------------------------------------------------------

RS485DMAClass RS485x(SerialX, TX_PIN, DE_PIN, RE_PIN);

// -----------------------------------------------------------------------------
// IRQ handlers
// -----------------------------------------------------------------------------

extern "C" void USARTx_IRQHandler(void)
{
    RS485x.usartIrqHandler();
}

extern "C" void DMAx_Streamz_IRQHandler(void)
{
    RS485x.txStreamIrqHandler();
}


// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup(){
    RS485x.setConfig(&RS485x_Config); //Confis MUST be called before begin()
    RS485x.begin(9600);

}

````


