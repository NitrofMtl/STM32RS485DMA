# Contributing to STM32RS485DMA

Thanks for your interest in contributing.

This library targets **STM32 DMA-based UART peripherals** and requires
a solid understanding of:

- STM32 HAL
- UART + DMA interaction
- Interrupt handling
- Cache coherency (DCache) on Cortex-M7 devices

Basic Arduino usage questions should go to **Discussions**, not Pull Requests.

---

# What we accept

Contributions are welcome for:

- Support for additional STM32 boards
- DMA / timing fixes
- Documentation improvements
- Examples
- HAL compatibility fixes

---

# What we do NOT accept (without discussion)

To preserve the design goals of this library, the following changes
will normally be rejected unless discussed first:

- Blocking APIs
- Polling-based RX/TX
- Non-DMA UART implementations
- Changes that break Arduino `Stream` semantics
- Features that introduce hidden background tasks

This library is intentionally **deterministic and interrupt-driven**.

---

# Library architecture overview

The core driver is **hardware-agnostic**.

Board support is implemented through **mapping tables** that associate:

- UART instance
- DMA streams
- IRQ numbers
- Default Serial pin mappings

The library automatically derives most configuration from:

- TX/RX pins
- Arduino `HardwareSerial`
- internal UART/DMA mapping tables

This greatly reduces the amount of board-specific code required.

---

# Adding support for a new board

Most STM32 boards can be supported by extending the mapping tables.

Typical steps:

1. Verify that the board exposes a **UART with DMA RX and TX**.

2. Add the UART entry to the internal **UART/DMA mapping table**, including:

   - UART instance (`USARTx`)
   - RX DMA stream
   - TX DMA stream
   - IRQ number
   - DMA request IDs (if required by the MCU family)

3. Ensure the board defines valid Arduino pin macros:

```cpp
SERIALx_TX
SERIALx_RX
```

These allow the driver to automatically detect the correct USART.

4. Test:

- RX DMA reception
- UART IDLE detection
- TX DMA transmission
- DE/RE timing

---

# Interrupt handling

STM32 interrupt symbols are defined by the MCU startup files and cannot
be registered dynamically.

For this reason IRQ handlers must **forward to the RS485DMA instance**.

Typical pattern:

```cpp
extern "C" void USART3_IRQHandler(void)
{
    RS485.usartIrqHandler();
}

extern "C" void DMA1_Stream3_IRQHandler(void)
{
    RS485.txStreamIrqHandler();
}

```

Handler names must match those defined by the STM32 startup files.

---

## STM32 family differences

When porting to a new MCU family, pay attention to:

- DMA vs BDMA vs GPDMA
- Presence or absence of DMAMUX
- UART FIFO support
- RCC clock tree differences
- Cache (DCache) requirements on Cortex-M7 devices
- HAL macro availability

Some differences may require conditional compilation.

---

## Code style guidelines

- No dynamic allocation
- Prefer explicit state machines
- Defensive HAL usage
- Avoid hidden side effects
- Avoid heavy work inside ISRs

The driver should remain **predictable and low-overhead**.

---

## Pull request guidelines

Please ensure that:

- Board-specific code is isolated
- No USART instances are hardcoded in core logic
- Existing boards (especially Opta) remain functional
- Arduino-style API compatibility is preserved

---


## Testing a new board

When validating support for a new board, two initialization methods
can be used.

### 1. Standard initialization (recommended)

Uses automatic USART detection and mapping.

```cpp
#include <RS485DMA.h>

RS485DMAClass RS485(Serial2, SERIAL2_TX, PB_14, PB_13);

void setup()
{
    RS485.begin(115200);
}

```

This is the preferred method and should work when the board
is supported by the internal mapping tables.

---

### 2. Explicit configuration (advanced testing)

When a board is not yet supported, contributors may manually
create a configuration object.

```cpp
#include <RS485DMA.h>

const RS485DMA_config* config =
    RS485DMA_config::fromPins(SERIAL2_TX, SERIAL2_RX);

RS485DMAClass RS485(config, SERIAL2_TX, PB_14, PB_13);

void setup()
{
    RS485.begin(115200);
}
```

This allows testing the driver before adding the board to the
internal mapping tables.

If this configuration works correctly, the board can then be
added to the library mappings.


### Modbus compatibility

This library supports multiple `RS485DMAClass` instances, allowing
several independent RS485 ports on the same board.

However, compatibility with STM32Modbus libraries requires that
one global instance is named `RS485`.

Example:

```cpp
RS485DMAClass RS485(Serial2, SERIAL2_TX, DE_PIN, RE_PIN);

```

Additional ports may be created with different names:

```
Additional ports may be created with different names:
```

If Modbus is used, the instance named RS485 must be declared
globally so that it can be referenced by the Modbus library.



### Testing expectations

- When adding support for a new board, contributors should verify:

- RX DMA reception

- TX DMA transmission

- UART IDLE detection

- DE / RE timing behavior

- No DMA overrun conditions