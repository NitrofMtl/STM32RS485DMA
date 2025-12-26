# STM32RS485DMA

High-performance RS485 library for STM32-based Arduino boards using DMA.

Designed for deterministic timing, low CPU usage, and industrial protocols
(Modbus RTU, custom binary frames, etc.).

---

## Features
- Full-duplex RS485 using DMA (TX + RX)
- RX idle detection using UART IDLE
- Deterministic DE/RE timing
- Arduino `Stream` compatible
- Compatible with Arduino RS485 API (where meaningful)

---

## Supported boards
- Arduino Opta (tested)
- STM32 boards with:
  - UART + DMA RX/TX
  - Optional DMAMUX (recommended)

> Other STM32 targets are expected to work but require board-specific configuration.

---

## Installation
- Arduino IDE: *Library Manager* (future)
- PlatformIO:
  ```ini
  lib_deps = nitrofmtl/STM32RS485DMA


## Basic usage

````
#include <RS485DMA.h>


// On Opta, the RS485 object is already created.
// Do NOT instantiate RS485DMAClass yourself.
void setup() {
    RS485.begin(115200);
}

void loop() {
    if (RS485.available()) {
        int b = RS485.read();
        RS485.write(b);
    }
}

````
## TX / RX usage model (important)

RX is enabled by default after `begin()`.

However, because this library uses DMA, RX and TX **cannot operate at the same time**.
When transmitting, RX DMA is temporarily disabled.

After a transmission completes, RX is **not automatically re-enabled**.
You must explicitly call `receive()` to resume reception.


### Typical sequence


````

// Read incoming data
while (RS485.available()) {
    buffer[i++] = RS485.read();
}

// Transmit response
RS485.noReceive();
RS485.beginTransmission();
RS485.write(response, len);
RS485.endTransmission();

// Resume RX
RS485.receive();

````

## Timing configuration

````
RS485.setDelays(predelay_us, postdelay_us);
RS485.setRxIdleTime(idle_time_us);

````
setDelays(predelay_us, postdelay_us)

Controls driver enable timing (same behavior as ArduinoRS485):

predelay: time (µs) to wait after asserting DE before sending data

postdelay: time (µs) to wait after the last byte before deasserting DE

These values depend on the RS485 transceiver and baud rate.

Same semantics as `ArduinoRS485.setDelays()`.

---

`setRxIdleTime(idle_time_us)` (DMA-specific)

Sets the additional idle time (in microseconds) required after the UART IDLE event
before received data is considered complete.

The UART hardware detects IDLE after approximately one character time (minimum, hardware-defined).
This function allows adding extra margin for noisy lines or custom protocols.

In most cases, the default value is sufficient and this function does not need to be used.

Typical usage:

Modbus RTU: ~3.5 character times

Custom protocols: protocol-dependent

Default value works for most cases

````
// Example: Modbus RTU at 115200 baud (~304 µs per char)
RS485.setRxIdleTime(1200);  // ≈ 4 characters
````

If unsure, you usually do not need to call this function.

## Important!

### ***RX and TX cannot occur simultaneously* (DMA constraint)**

