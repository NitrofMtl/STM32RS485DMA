# STM32 DMA stream selection

## H7 (DMAMUX present)

Streams are flexible. Any free stream can be used.

The library automatically configures the DMAMUX request.

User must only ensure the stream is not used elsewhere.

## F4 / F7 (no DMAMUX)

Streams are constrained by the hardware.

Each UART supports a limited set of streams.
The correct combinations are listed in the STM32 reference manual
under "DMA request mapping".

## STM32F2/F4/F7 DMA Stream Selection

For STM32F2/F4/F7 devices, UART peripherals are connected to specific DMA streams.

The STM32RS485DMA library automatically configures the correct DMA channel
for each USART/UART peripheral.

Therefore users only need to select a DMA stream compatible with that
peripheral according to the STM32 reference manual.

### From [Application note AN4031](https://www.st.com/resource/en/application_note/an4031-using-the-stm32f2-stm32f4-and-stm32f7-series-dma-controller-stmicroelectronics.pdf), Table 1 and 2:

- USART1_RX: DMA2_STREAM2, DMA2_STREAM5
- USART1_TX: DMA2_STREAM7

- USART2_RX: DMA1_STREAM5
- USART2_TX: DMA1_STREAM6

- USART3_RX: DMA1_STREAM1
- USART3_TX: DMA1_STREAM3

- UART4_RX: DMA1_STREAM2
- UART4_TX: DMA1_STREAM4

- UART5_RX: DMA1_STREAM0
- UART5_TX: DMA1_STREAM7

- USART6_RX: DMA2_STREAM1, DMA2_STREAM2
- USART6_TX: DMA2_STREAM6, DMA2_STREAM7

- UART7_RX: DMA1_STREAM3
- UART7_TX: DMA1_STREAM1

- UART8_RX: DMA1_STREAM6
- UART8_TX: DMA1_STREAM0


1️⃣ F Series (F2/F4/F7)

Each USART is hardwired to certain DMA streams.

Some streams are shared between different USARTs.
Example (F4 RM0433 / RM0008):

USART	RX Stream	TX Stream
USART1	DMA2_Stream2 / 5	DMA2_Stream7
USART2	DMA1_Stream5	DMA1_Stream6
USART3	DMA1_Stream1	DMA1_Stream3
…	…	…

Implication:
You cannot simultaneously use USARTs that share the same DMA stream for DMA transfers. If you try, the transfers will collide.

Recommendation: only instantiate one RS485DMAClass per stream at a time.

2️⃣ H7 Series

Streams are flexible, but there are fewer DMA streams than USARTs.

Default mapping is provided for USART1–8.

If a user wants to use more than 8 USARTs or reuse streams, they must:

Check which streams are free.

Override streams using fromPins(tx, rx, rxStream, txStream).

Ensure no collisions with other peripherals.

Implication:
RS485DMA cannot automatically guarantee conflict-free operation for H7 beyond the default mapping. Users are responsible for validation.





### 🔹 1. What is a DMA conflict?

A DMA conflict occurs when two peripherals attempt to use the same DMA stream simultaneously.

Each DMA stream can only serve one peripheral at a time.

### 🔹 2. Why it happens
DMA resources are limited and shared across peripherals.

The library selects default streams automatically, but other peripherals
(SPI, ADC, additional UARTs, etc.) may also use DMA.

### 🔹 3. Symptoms of a conflict

Common symptoms include:

- No data received (RX never triggers)
- Transmission never completes
- Corrupted or partial data
- Interrupt handlers not firing
- HAL state stuck (busy / not ready)

### 🔹 4. Why it is not detected automatically
The STM32 HAL does not provide a reliable way to detect DMA stream conflicts at runtime.

Conflicts depend on the full application, including other libraries and peripherals.

### 🔹 5. How to fix it
Use the advanced `fromPins()` variant to manually assign DMA streams:

```cpp
RS485DMA_config cfg = RS485DMA_config::fromPins(
    txPin,
    rxPin,
    rxStream,
    txStream
);

```

Select streams that are not used by other peripherals.


---

### 🔹 6. Practical advice

- Start with default configuration
- If issues appear, try different DMA streams
- Avoid reusing streams already used by SPI, ADC, or other UARTs

