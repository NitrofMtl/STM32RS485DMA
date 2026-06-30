# Changelog

---
## [0.9.1] - 2026-06-28

### Added
-Tx complete callback function pointer

### Fix
-NoReceive idle handleing

---
## [0.9.0] - 2026-04-12

### Added
-added IRQ callbacks support

---

## [0.8.0] - 2026-03-25

### Added
- Now compile on (Still need to be tested):
  - nucleo_f446re
  - nucleo_f401re
  - nucleo_f767zi
  - nucleo_f722ze
  - nucleo_h743zi
- IRQHandlers identification helper `checkIrqHandlers()`

### Changed
- now only constructor with `(const RS485DMA_config* config, PinName dePin, PinName rePin)` is valid

---



## [0.7.0] - 2026-03-11

### Added
- Automatic USART instance detection from TX/RX pins.
- Automatic GPIO configuration generation for UART pins.
- Serial-to-pin mapping support (`fromSerial()` helper).
- Config validation helper (`hasValidConfig()`).
- Optional manual DMA stream configuration for unsupported boards.
- Debug checks to verify expected IRQ handlers at runtime.

### Changed
- Simplified `RS485DMA_config` structure: USART instance can now be derived from pins.
- Reduced required board-specific configuration.
- Improved portability across STM32 boards.

### Fixed
- Correct handling of `const` configuration objects.
- Safer GPIO initialization for UART pins.
- Improved compile compatibility with different STM32 cores.

### Internal
- Refactored UART/DMA mapping logic.
- Improved error handling when configuration is invalid.

---

## [0.6.2]
### Change
- correct timing guard


## [0.6.0] - 2026-01-17

### Added
- Frame-based RX API using UART IDLE detection (`readFrame()`)

### Fixed
- RX buffer desynchronization when mixing DMA reception with RXNE-based activity detection


---


## [0.5.1] - 2026-01-05
### Added
- STM32F4 support (DMA + UART clock handling)
- Generic DMA clock enable (DMA1/DMA2/BDMA)

### Changed
- Removed dependency on STM32H7xx build flag
- Unified HAL clock enabling logic

---

## [0.5.0] - 2025-12-25
### Initial commit

---
