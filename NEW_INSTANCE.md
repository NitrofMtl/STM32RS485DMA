# Library architecture overview

The driver is hardware-agnostic and automatically configures itself from TX/RX pins.

These tables associate:

- USART instance
- DMA streams (or default allocation)
- IRQ numbers
- DMA request or channel information

When calling:
---
```
RS485DMA_config cfg = RS485DMA_config::fromPins(txPin, rxPin);
```
the library will:

- Resolve the USART peripheral from the TX pin
- Match it with the corresponding RX pin
- Select appropriate DMA streams and configuration
- Associate the correct IRQ handlers

This design minimizes board-specific setup and allows most configurations to work without manual intervention.

Advanced users can override DMA stream selection using the extended `fromPins()` variant if needed.

## Pin requirements

`RS485DMA_config::fromPins()` relies on STM32 pin mapping and expects valid `PinName` values.

- Pins must be valid STM32 pins supported by the board variant.
- Pins must be compatible with a UART/USART peripheral (TX/RX alternate function).
- The mapping is resolved internally using `pinmap_peripheral()`.

Example:

```cpp
RS485DMA_config cfg = RS485DMA_config::fromPins(PA_9, PA_10);
```

This allows the driver to automatically detect the USART instance and build the correct configuration object.

Note: If an invalid pin is provided, the configuration will be marked invalid (all fields set to zero).
Both TX and RX pins must map to the same USART peripheral.
If the configuration is not valid, `begin()` will return false;

You can validate the configuration:

```
RS485.hasValidConfig();
```



#### Arduino pin names

On STM32 Arduino cores, pins can be expressed as:

- STM32-style: `PA_9`, `PB_7`, etc. (recommended)
- Arduino-style (e.g. `D1`, `A0`) if defined by the variant

Internally, all pins are converted to `PinName`.



## Example

```cpp
#include <RS485DMA.h>

constexpr PinName txPin = PA_2;
constexpr PinName rxPin = PA_3;

constexpr PinName dePin = PB_14;
constexpr PinName rePin = PB_13;

RS485DMA_config cfg = RS485DMA_config::fromPins(txPin, rxPin);
RS485DMAClass RS485(&cfg, dePin, rePin);

void setup()
{
    RS485.begin(115200);
    if (!RS485.hasValidConfig()) {
        //handle error
        while(1);
    }
}
```


## Interrupt handling

STM32 cores differ in how UART events can be intercepted:

Some cores expose weak IRQ handlers (e.g. PlatformIO / STM32Duino)
Others expose weak HAL callbacks instead (e.g. Arduino IDE STM32 core)

Because of this, the library supports two integration methods.
You only need to use one, depending on your platform.

> **Important:** Handler names must exactly match the symbols defined in the STM32 startup files, and must be declared with `extern "C"`.

### Option A — IRQ forwarding (recommended when available)

If your core allows overriding IRQ handlers, forward them to the library:
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


Each UART/DMA used by your configuration must have its handler defined.

To determine which handlers are required for your configuration, you can call the following in `setup()`:
```cpp
RS485.checkIrqHandlers();
```

This will print the exact handler names to add.

## Option B — HAL callback forwarding

If IRQ handlers are not overridable, use HAL callbacks instead:

```cpp
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    RS485DMAClass::TxCpltCallback(huart);
}

extern "C" void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    RS485DMAClass::RxEventCallback(huart, size);
}

extern "C" void DMA1_Stream3_IRQHandler(void)
{
    RS485.txStreamIrqHandler();
}
```

To determine which stream handlers are required for your configuration, you can also call the following in `setup()`:
```cpp
RS485.checkIrqHandlers();
```
When using callbacks, USART IRQ handlers are not required, but
DMA stream IRQ handlers must still be defined.

---

Unlike IRQ handlers, only one declaration is needed, as the callback
receives the UART instance and dispatches it internally.

---


⚠️ Callback conflicts

HAL callbacks are global functions.
If another library also defines them, only one implementation can exist.

In that case, you must manually dispatch to all libraries:

```cpp
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    RS485DMAClass::TxCpltCallback(huart);
    OtherLib_TxCallback(huart); // if needed
}
```

The same applies to RX callbacks.

---

Choosing the right method:
- If compilation fails when defining IRQ handlers → use callbacks
- If callbacks are not called or cannot be overridden → use IRQ forwarding


Summary:

- IRQ method → one handler per peripheral
- Callback method → single global handler
- Both paths are fully supported and equivalent internally

---


## Advanced configuration (DMA stream override)

In most cases, `fromPins(tx, rx)` is sufficient and automatically selects appropriate DMA streams.

However, each DMA stream can only be used by one peripheral at a time. If another peripheral is already using a selected stream, it will lead to conflicts and undefined behavior.

In that case, users may override the DMA stream selection using the extended variant:

```cpp
RS485DMA_config cfg = RS485DMA_config::fromPins(
    PA_2,
    PA_3,
    DMA1_Stream1,
    DMA1_Stream2
);
```


### Modbus compatibility
---

This library supports multiple `RS485DMAClass` instances, allowing
several independent RS485 ports on the same board.

However, compatibility with STM32Modbus libraries requires that
one global instance is named `RS485`.

Example:

```cpp
RS485DMAClass RS485(&cfg, dePin, rePin);

```

Additional ports may be created with different names:

If Modbus is used, the instance named RS485 must be declared
globally so that it can be referenced by the Modbus library.



