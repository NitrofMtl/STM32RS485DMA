# Changelog

---

## [0.6.0] - 2026-01-17
### Added
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
