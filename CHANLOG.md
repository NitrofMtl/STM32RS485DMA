# Changelog

---

## [Unreleased]

### Added
- Planned default pin mappings for:
  - Arduino GIGA R1
  - Arduino Portenta
  - Arduino Machine Control

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
