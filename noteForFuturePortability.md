# Portability Notes

This library was developed and validated on STM32H7 (Arduino Opta).

While most STM32 families share UART/DMA concepts, important differences exist.

---

## Known compatible architectures
- STM32H7 (tested)
- STM32G4 (expected)
- STM32F4 / F7 (with cache handling adjustments)

---

## Potential problem areas

### 1. DMA controller differences
- Stream-based (F4/F7/H7)
- Channel-based (G0/G4/L4)
- DMAMUX availability

### 2. Cache coherency
- Cortex-M7 requires explicit DCache maintenance
- Cortex-M4 may not

### 3. UART feature differences
- FIFO support
- IDLE flag behavior
- Break signaling availability

### 4. Arduino core abstraction gaps
- Missing AF pinMode()
- Inconsistent USART naming across boards

---

## IRQ handling strategy

This library does not hardcode weak IRQ handlers.

Board support must explicitly bind:
- USART IRQ
- DMA RX IRQ
- DMA TX IRQ

This avoids conflicts with other libraries.

---

## Open design questions
- Generic DMAMUX abstraction
- Unified pin-to-peripheral mapping
- Optional HAL-less backend



---
---

## Portability and architecture support

RS485DMA is designed for STM32 microcontrollers using the STM32 HAL
with UART peripherals driven by DMA.

### Validated
- Arduino Opta (STM32H7) USART3

### Expected to work with configuration
- STM32U5
- STM32G4
- STM32F7

### Experimental / community validated
- STM32F4
- STM32L4 / L4+
- STM32WB / WL
- STM32F3

### Not currently targeted
- STM32F1 / F0 / L0 / C0
- Non-STM32 platforms

Porting to additional STM32 families typically requires only
DMA request mapping and IRQ configuration, not changes to core logic.

