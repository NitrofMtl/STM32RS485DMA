#include "STM32RS485DMA.h"
#include "board_config/board_config.h"


RS485DMAClass::RS485DMAClass(HardwareSerial& serial, PinName txPin, PinName dePin, PinName rePin)
: _config(getRS485DMAConfig(serial)), _txPin(txPin), _dePin(dePin), _rePin(rePin)
{
    if (!_config) {
        // No valid DMA config found for this Serial instance → stop here
        while (1) {
            // Hard fail: hang here so the dev notices the bug
            // Could also blink LED_BUILTIN instead of blocking
        }
    }
}


bool RS485DMAClass::begin(unsigned long baudrate, uint16_t config, int predelay, int postdelay) {
    if (baudrate <= 0) return false;

    if (!_config) return false; // explicit failure

    setDelays(predelay, postdelay);

    if (_dePin != NC) {
        pinMode(_dePin, OUTPUT);
        digitalWrite(_dePin, LOW); // default driver disabled
    }
    if (_rePin != NC) {
        pinMode(_rePin, OUTPUT);
        digitalWrite(_rePin, LOW); // default receiver enabled
    }

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    memset((void*)_dma_rx_buffer, 0, DMA_RX_BUFFER_SIZE); // clear junk

    setupUsart(baudrate);

    if (!initDMA(config)) {
        return false;
    }

    setRxIdleTime(getUsecForNChar(DEFAULT_CHAR_TIME));

    // Default to RX mode
    receive();
    _begun = true;

    delay(5);
    return true;
}


void RS485DMAClass::end()
{
    isRxIdle();
    DMATxTimeOut();
    digitalWrite(_dePin, LOW);
    digitalWrite(_rePin, LOW);
    HAL_UART_AbortReceive(&_huart);
    HAL_UART_AbortTransmit(&_huart);
    HAL_DMA_DeInit(&_hdma_rx);
    HAL_DMA_DeInit(&_hdma_tx);
    HAL_UART_DeInit(&_huart);
    _rxTail = 0;
    _begun = false;
}


RS485DMAClass::operator bool()
{
    return _begun;
}


int RS485DMAClass::available()
{
    size_t head = _rxHead;
    size_t tail = _rxTail;

    if (head == tail)
        return 0;

    // ---------- Return total available bytes ----------
    if (head > tail) {
        return head - tail;
    } else {
        return (DMA_RX_BUFFER_SIZE - tail) + head;
    }
}


int RS485DMAClass::read()
{
    if (!available()) return -1;

    invalidateRxCache(_rxTail, 1);
 
    uint8_t byte = _dma_rx_buffer[_rxTail];
    _rxTail = (_rxTail +1) % DMA_RX_BUFFER_SIZE;
    return byte;
}


int RS485DMAClass::readFrame(uint8_t* buffer, size_t bufferSize)
{
    if (!_frame.armed) {
        return 0;
    }

    if ((micros() - _frame.idleTimeStamp) < _postDelay) { // wait post delay
        return 0;
    }

    size_t len = min(bufferSize, _frame.len);

    if (len + _rxTail <= DMA_RX_BUFFER_SIZE) {
        // contiguous
        invalidateRxCache(_rxTail, len);
        memcpy(buffer, (uint8_t*)&_dma_rx_buffer[_rxTail], len);
        
    } else {
        // wrapped around
        size_t firstPart = DMA_RX_BUFFER_SIZE - _rxTail;
        invalidateRxCache(_rxTail, firstPart);
        memcpy(buffer, (u_int8_t*)&_dma_rx_buffer[_rxTail], firstPart);
        invalidateRxCache(0, len - firstPart);
        memcpy(buffer + firstPart, (u_int8_t*)&_dma_rx_buffer[0], len - firstPart);
    }
    _rxTail = (_rxTail + len) % DMA_RX_BUFFER_SIZE;
    _frame.len -= len;

    if (_frame.len == 0) {
        // Frame fully consumed
        _frame.armed = false;
        if (_frame.overflow) {
            _rxTail = dma_rx_head();  // discard only AFTER frame done
            _frame.overflow = false;
            //Serial.println("overflow"); helper on debugging
        }
    }
    return len;
}


void RS485DMAClass::flush()
{
   uint32_t start = micros();
    while (!isRxIdle()) {
        if (micros() - start > RS485DMA_TX_GUARD_US) {
            // Timeout waiting for bus to go idle
            return;
        }
    }
}


int RS485DMAClass::peek()
{
    if (!available()) return -1;
    return _dma_rx_buffer[_rxTail];
}


void RS485DMAClass::setDelays(uint32_t predelay, uint32_t postdelay)
{
    _preDelay  = predelay;
    _postDelay = postdelay;
}


void RS485DMAClass::setPins(int txPin, PinName dePin, PinName rePin)
{
    //tx ignored, kept fot arduinoRS485 lib compatibility
    _dePin = dePin;
    _rePin = rePin;
}


void RS485DMAClass::beginTransmission()
{
    // Enable DE/RE lines
    if (_dePin != NC) digitalWrite(_dePin, HIGH);
    if (_rePin != NC) digitalWrite(_rePin, HIGH);

    if (_preDelay) delayMicroseconds(_preDelay);

    __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_TCF | UART_CLEAR_TXFECF);
}


void RS485DMAClass::endTransmission()
{
    if (!DMATxTimeOut()) return;

     // --- fully abort TX and reset HAL state ---
    HAL_UART_AbortTransmit(&_huart);

    if (_postDelay) delayMicroseconds(_postDelay);

    // Disable transmitter
    if (_dePin >= 0) digitalWrite(_dePin, LOW);
    if (_rePin >= 0) digitalWrite(_rePin, LOW);

    // Clear TC flag (if needed) – safe to call
    __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_TCF | UART_CLEAR_IDLEF | UART_CLEAR_OREF | UART_CLEAR_NEF);

}


void RS485DMAClass::receive()
{
    // stop any HAL-level RX to be sure the handle is clean
    HAL_UART_AbortReceive(&_huart);

#if defined(USART_ICR_FECF)
    USART3->ICR = USART_ICR_FECF | USART_ICR_ORECF | USART_ICR_IDLECF | USART_ICR_RTOCF
                  | USART_ICR_PECF | USART_ICR_TCCF; // clear framing/overrun/idle/timeout/Noise/Parity
#else
    // Fallback: clear common flags using HAL macro
    __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_FEF | UART_CLEAR_OREF | UART_CLEAR_IDLEF | UART_CLEAR_RTOF | UART_CLEAR_PEF);
#endif

    // arm RX DMA if it's not already running
    auto inst = static_cast<DMA_Stream_TypeDef*>(_hdma_rx.Instance);
    if ((inst->CR & DMA_SxCR_EN) == 0) {
        if (HAL_UART_Receive_DMA(&_huart, (uint8_t*)_dma_rx_buffer, DMA_RX_BUFFER_SIZE) != HAL_OK) {
            Serial.println("[RS485LIB] HAL_UART_Receive_DMA failed");
            // try to recover: abort + clear flags + return
            HAL_UART_AbortReceive(&_huart);
            return;
        }
    }

    //update rx tail pointer based on DMA counter (safe now that DMA armed)
    _rxTail = DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(_huart.hdmarx);

    __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_IDLEF);
    __HAL_UART_ENABLE_IT(&_huart, UART_IT_IDLE);
}


void RS485DMAClass::noReceive()
{
    flush();

    __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_IDLEF);
    __HAL_UART_DISABLE_IT(&_huart, UART_IT_IDLE);
    // Stop DMA safely
    auto inst = static_cast<DMA_Stream_TypeDef*>(_hdma_rx.Instance);
    __HAL_DMA_DISABLE(&_hdma_rx);
    if (inst->CR & DMA_SxCR_EN) {       
        while ((inst->CR & DMA_SxCR_EN) != 0) { /// this could be guarded
            // Wait for DMA to fully disable
            yield();
        }
    }
}


uint32_t RS485DMAClass::getUsecForNChar(float n)
{
   float bits = 1.0f; // Start bit always present

    // --- Word length ---
    switch (_huart.Init.WordLength) {
#if defined(UART_WORDLENGTH_6B)
        case UART_WORDLENGTH_6B: bits += 6.0f; break;
#endif
#if defined(UART_WORDLENGTH_7B)
        case UART_WORDLENGTH_7B: bits += 7.0f; break;
#endif
        case UART_WORDLENGTH_8B: bits += 8.0f; break;
        case UART_WORDLENGTH_9B: bits += 9.0f; break;
        default: bits += 8.0f; break; // fallback
    }

    // --- Parity bit (always transmitted even if not stored) ---
    if (_huart.Init.Parity != UART_PARITY_NONE)
        bits += 1.0f;

    // --- Stop bits ---
    switch (_huart.Init.StopBits) {
        case UART_STOPBITS_0_5: bits += 0.5f; break;
        case UART_STOPBITS_1:   bits += 1.0f; break;
        case UART_STOPBITS_1_5: bits += 1.5f; break;
        case UART_STOPBITS_2:   bits += 2.0f; break;
        default: bits += 1.0f; break; // fallback
    }

    return bits * 1000000.0f * n / _huart.Init.BaudRate;
}


size_t RS485DMAClass::write(uint8_t b)
{
    return write(&b, 1);
}


size_t RS485DMAClass::write(const uint8_t *buffer, size_t size)
{
    if (size == 0) return 0;

    size_t written = 0;

    while (size > 0) {
        if (!DMATxTimeOut()) {
            Serial.println("[RS485LIB] TX DMA timeout — aborting previous transfer");
            return written;
        }
        size_t chunk = (size > DMA_TX_BUFFER_SIZE) ? DMA_TX_BUFFER_SIZE : size;
        memcpy((void*)_dma_tx_buffer, buffer + written, chunk);

        written += chunk;
        size = (size > chunk) ? size-chunk : 0;

        startNextTxChunk(chunk); 
    }

    return written;
}


void RS485DMAClass::sendBreak(uint32_t duration)
{
    uint32_t start = millis();
    constexpr uint64_t timeout = 2000;/// delay could be adusjt ->> like 3.5char for modbus...
    while(!isRxIdle()) { //ensure RX is idle
        if (millis()  - start > timeout) break;
        yield();
    }
    DMATxTimeOut();           // ensure TX DMA idle

    // Assert DE and disable RX (active low)
    digitalWrite(_dePin, HIGH);
    digitalWrite(_rePin, HIGH);

    // Take UART TX pin into GPIO output low
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, LOW);

    start = millis();
    while (millis() - start < duration) {
        yield();
    }
    // Restore UART TX function
    GPIO_InitTypeDef gi = _config->GPIO;
    HAL_GPIO_Init(_config->gpio_port, &gi);
    // Deassert DE and disable RX (active low)
    digitalWrite(_dePin, LOW);
    digitalWrite(_rePin, LOW);
}


void RS485DMAClass::sendBreakMicroseconds(uint32_t duration)
{
    uint32_t start = millis();
    constexpr uint64_t timeout = 2000;/// delay could be adusjt ->> like 3.5char for modbus...
    while(!isRxIdle()) { //ensure RX is idle
        if (millis()  - start > timeout) break;
        yield();
    }
    DMATxTimeOut();           // ensure TX DMA idle

    // Assert DE and disable RX (active low)
    digitalWrite(_dePin, HIGH);
    digitalWrite(_rePin, HIGH);

    // Take UART TX pin into GPIO output low
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, LOW);

    start = micros();
    while (micros() - start < duration) {
        yield();
    }
    // Restore UART TX function
    GPIO_InitTypeDef gi = _config->GPIO;
    HAL_GPIO_Init(_config->gpio_port, &gi);
    // Deassert DE and disable RX (active low)
    digitalWrite(_dePin, LOW);
    digitalWrite(_rePin, LOW);
}

void RS485DMAClass::setConfig(const RS485DMA_config* cfg)
{
    if (_begun) return; // or assert
    _config = cfg;
}

bool RS485DMAClass::hasValidConfig() const {
    return _config != nullptr;
}

void RS485DMAClass::startNextTxChunk(size_t size)
{
    // Ensure UART state ready (best-effort)
    if (_huart.gState != HAL_UART_STATE_READY) {
        // make sure no DMA TX still active
        if ((static_cast<DMA_Stream_TypeDef*>(_hdma_tx.Instance)->CR & DMA_SxCR_EN) != 0) {
            __HAL_DMA_DISABLE(&_hdma_tx);
            while ((static_cast<DMA_Stream_TypeDef*>(_hdma_tx.Instance)->CR & DMA_SxCR_EN) != 0);
        }
        // reset HAL state
        _huart.gState = HAL_UART_STATE_READY;
    }

    cleanTxDCache(size);
    
    __HAL_DMA_CLEAR_FLAG(&_hdma_tx,
    __HAL_DMA_GET_TC_FLAG_INDEX(&_hdma_tx) |
    __HAL_DMA_GET_HT_FLAG_INDEX(&_hdma_tx) |
    __HAL_DMA_GET_TE_FLAG_INDEX(&_hdma_tx));

    // Start DMA transmission
    if (HAL_UART_Transmit_DMA(&_huart, (uint8_t*)_dma_tx_buffer, size) != HAL_OK) {
        Serial.println("[RS485LIB] HAL_UART_Transmit_DMA failed");
        _txBusy = false;
        return;
    }

    // disable DMA Half-Transfer interrupt
    __HAL_DMA_DISABLE_IT(&_hdma_tx, DMA_IT_HT | DMA_IT_TE | DMA_IT_DME);

    _txBusy = true;

    // Enable the Transmission Complete interrupt
    __HAL_UART_ENABLE_IT(&_huart, UART_IT_TC);
}


void RS485DMAClass::invalidateRxCache(size_t offset, size_t length)
{
    if (length == 0) return;

    constexpr size_t CACHE_LINE = 32;

    size_t firstPart = min(length, DMA_RX_BUFFER_SIZE - offset);

    // ---- First segment ----
    uintptr_t startPtr = (uintptr_t)&_dma_rx_buffer[offset];
    uintptr_t aligned_start = startPtr & ~(CACHE_LINE - 1);
    uintptr_t aligned_end =
        ((startPtr + firstPart + CACHE_LINE - 1) & ~(CACHE_LINE - 1));

    SCB_InvalidateDCache_by_Addr(
        (uint32_t*)aligned_start,
        aligned_end - aligned_start
    );

    // ---- Wrapped segment ----
    if (firstPart < length) {
        size_t secondPart = length - firstPart;
        startPtr = (uintptr_t)&_dma_rx_buffer[0];
        aligned_start = startPtr & ~(CACHE_LINE - 1);
        aligned_end =
            ((startPtr + secondPart + CACHE_LINE - 1) & ~(CACHE_LINE - 1));

        SCB_InvalidateDCache_by_Addr(
            (uint32_t*)aligned_start,
            aligned_end - aligned_start
        );
    }
}



void RS485DMAClass::cleanTxDCache(size_t len)
{
    constexpr size_t CACHE_LINE = 32;

    uintptr_t start = (uintptr_t)_dma_tx_buffer;
    uintptr_t aligned_start = start & ~(CACHE_LINE - 1);

    uintptr_t end = start + len;
    uintptr_t aligned_end = (end + CACHE_LINE - 1) & ~(CACHE_LINE - 1);

    SCB_CleanDCache_by_Addr(
        (uint32_t*)aligned_start,
        aligned_end - aligned_start
    );
}


bool RS485DMAClass::DMATxTimeOut()
{
    constexpr uint32_t TX_TIMEOUT_MARGIN_CHARS = 5;
    uint32_t timeout = getUsecForNChar(DMA_TX_BUFFER_SIZE / 2 + TX_TIMEOUT_MARGIN_CHARS);
    uint32_t start = micros();
    while (_txBusy) {
        if (micros() - start > timeout) {
            HAL_DMA_Abort(&_hdma_tx);
            _txBusy = false;
            Serial.println("[RS485LIB] Tx timed out!!!");
            return false;
        }
        yield();
    }
    return true;
}


void RS485DMAClass::setupUsart(uint32_t baudrate)
{
    // Reset DMAMUX if your MCU uses it
    #if defined(__HAL_RCC_DMAMUX1_FORCE_RESET)
    __HAL_RCC_DMAMUX1_FORCE_RESET();
    __HAL_RCC_DMAMUX1_RELEASE_RESET();
    #endif

    // clocks
    RS485DMA_EnableGPIOClock(_config->gpio_port); // assuming TX/RX pins on port B
    RS485DMA_EnableUARTClock(_config->instance);

    GPIO_InitTypeDef gi = _config->GPIO;
    HAL_GPIO_Init(_config->gpio_port, &gi);

    _huart.Instance = _config->instance;
    _huart.Init.BaudRate = baudrate;
    _huart.Init.WordLength = UART_WORDLENGTH_8B;
    _huart.Init.StopBits = UART_STOPBITS_1;
    _huart.Init.Parity = UART_PARITY_NONE;
    _huart.Init.Mode = UART_MODE_TX_RX;
    _huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    _huart.Init.OverSampling = UART_OVERSAMPLING_16;
    _huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    _huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    _huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    //_huart.FifoMode = UART_FIFOMODE_DISABLE;//make rx bug

    if (HAL_UART_Init(&_huart) != HAL_OK) {
        Serial.println("[RS485LIB] HAL UART Init failed");
        //Error_Handler();
    }

}


bool RS485DMAClass::initDMA(uint16_t config)
{
    // ---------- RX DMA SETUP ----------
    RS485DMA_EnableDMAClock();
    // Configure DMA handle
    _hdma_rx.Instance = _config->rx.stream;//dmaRxStream;
    _hdma_rx.Init.Request = _config->rx.request;//dmaRxRequest;
    _hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    _hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    _hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
    _hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    _hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    _hdma_rx.Init.Mode = DMA_CIRCULAR;
    _hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;
    _hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&_hdma_rx) != HAL_OK) {
        Serial.println("[RS485LIB] HAL DMA Init failed");
        return false;
    }
     
    // Link DMA handle to UART
    __HAL_LINKDMA(&_huart, hdmarx, _hdma_rx);

    HAL_NVIC_SetPriority(_config->usart_irqn, 1 ,0);//UsartIRQnRx, 1, 0);
    HAL_NVIC_EnableIRQ(_config->usart_irqn);//UsartIRQnRx);

    // ---------- TX DMA SETUP ----------
    _hdma_tx.Instance = _config->tx.stream;//dmaTxStream;
    _hdma_tx.Init.Request = _config->tx.request;//dmaTxRequest;
    _hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    _hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    _hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
    _hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    _hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    _hdma_tx.Init.Mode = DMA_NORMAL;
    _hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
    _hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    HAL_UART_AbortTransmit(&_huart);

    if (HAL_DMA_Init(&_hdma_tx) != HAL_OK) {
        Serial.println("[RS485LIB] HAL TX DMA Init failed");
        return false;
    }

    //TX stream IRQ handler
    //HAL_NVIC_SetPriority(_config->IRQnRxStream, 0, 0); //////////not used for now...
    //HAL_NVIC_EnableIRQ(_config->IRQnRxStream);

    // Link TX DMA handle
    __HAL_LINKDMA(&_huart, hdmatx, _hdma_tx);

    // Enable NVIC for DMA TX stream
    HAL_NVIC_SetPriority(_config->tx.irqn, 5, 0);//IRQnTxStream, 5, 0);
    HAL_NVIC_EnableIRQ(_config->tx.irqn);//IRQnTxStream);

    return true;
}


void RS485DMAClass::onRxIdleIRQ()
{ 
    _rxHead = dma_rx_head();
    _lastRxTimeStamp = micros();

    if (_frame.armed && !( _lastRxTimeStamp - _frame.idleTimeStamp > _postDelay)) {
        _frame.overflow = true;
        return;
    }

    _frame.head = _rxHead;
    size_t len = (_frame.head - _rxTail + DMA_RX_BUFFER_SIZE) % DMA_RX_BUFFER_SIZE;
    if (len == 0) return;   // ignore spurious IDLE
    _frame.len = len;
    _frame.idleTimeStamp = _lastRxTimeStamp;
    _frame.armed = true;
}


void RS485DMAClass::onTxComplete()
{
    _txBusy = false;
}


void RS485DMAClass::usartIrqHandler()
{
       // --- TC: transmission complete ---
    if (__HAL_UART_GET_FLAG(&_huart, UART_FLAG_TC) &&
        __HAL_UART_GET_IT_SOURCE(&_huart, UART_IT_TC)) {
        __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_TCF);
        __HAL_UART_DISABLE_IT(&_huart, UART_IT_TC);
        RS485.onTxComplete();
        // Defer rest to HAL (calls callbacks, handles TC, RXNE if needed)
        //HAL_UART_IRQHandler(&RS485._huart);
        return;
    }

    if (__HAL_UART_GET_FLAG(&_huart, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_FLAG(&_huart, UART_CLEAR_IDLEF);
        //__HAL_UART_DISABLE_IT(&_huart, UART_IT_IDLE);
        RS485.onRxIdleIRQ();
    }
}


void RS485DMAClass::txStreamIrqHandler()
{
    HAL_DMA_IRQHandler(&_hdma_tx);
}


bool RS485DMAClass::isRxIdle()
{
    if (_rxHead != dma_rx_head()) {
        return false;
    }

    uint32_t elapsed = micros() - _lastRxTimeStamp;
    if (elapsed < _rxIdleTime) return false;

    return true;
}


void RS485DMAClass::setRxIdleTime(uint32_t duration)
{
    uint32_t oneCharTime = getUsecForNChar(1);//idle IRQ take one char
    if (duration <= oneCharTime) {
        _rxIdleTime = 0;
        return;
    }
    duration -= oneCharTime;
    _rxIdleTime = duration;
}


float RS485DMAClass::getBitsPerChar()
{
    float bits = 1.0f; // Start bit always present

    // --- Word length ---
    switch (_huart.Init.WordLength) {
#if defined(UART_WORDLENGTH_6B)
        case UART_WORDLENGTH_6B: bits += 6.0f; break;
#endif
#if defined(UART_WORDLENGTH_7B)
        case UART_WORDLENGTH_7B: bits += 7.0f; break;
#endif
        case UART_WORDLENGTH_8B: bits += 8.0f; break;
        case UART_WORDLENGTH_9B: bits += 9.0f; break;
        default: bits += 8.0f; break; // fallback
    }

    // --- Parity bit (always transmitted even if not stored) ---
    if (_huart.Init.Parity != UART_PARITY_NONE)
        bits += 1.0f;

    // --- Stop bits ---
    switch (_huart.Init.StopBits) {
        case UART_STOPBITS_0_5: bits += 0.5f; break;
        case UART_STOPBITS_1:   bits += 1.0f; break;
        case UART_STOPBITS_1_5: bits += 1.5f; break;
        case UART_STOPBITS_2:   bits += 2.0f; break;
        default: bits += 1.0f; break; // fallback
    }

    return bits;
}

#ifdef ARDUINO_OPTA
#define RS485_USING_USART3
#define RS485_USING_DMA1_Stream1
RS485DMAClass RS485(RS485_OPTA_DEFAULT_PINS);
#endif

#ifdef RS485_USING_USART3
extern "C" void USART3_IRQHandler(void) {
    RS485.usartIrqHandler();
}
#endif

#ifdef RS485_USING_DMA1_Stream1
extern "C" void DMA1_Stream1_IRQHandler(void)
{
    RS485.txStreamIrqHandler();
}
#endif



