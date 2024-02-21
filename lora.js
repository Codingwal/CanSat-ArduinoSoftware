const
    RH_RF95_MODE_SLEEP = 0x00,
    RH_RF95_REG_01_OP_MODE = 0x01,
    RH_RF95_LONG_RANGE_MODE = 0x80,
    RH_RF95_REG_06_FRF_MSB = 0x06,
    RH_RF95_REG_07_FRF_MID = 0x07,
    RH_RF95_REG_08_FRF_LSB = 0x08,
    RH_RF95_REG_0E_FIFO_TX_BASE_ADDR = 0x0e,
    RH_RF95_REG_0F_FIFO_RX_BASE_ADDR = 0x0f,
    RH_RF95_HEADER_LEN = 4,
    RH_RF95_REG_4D_PA_DAC = 0x4d,
    RH_RF95_PA_DAC_ENABLE = 0x07,
    RH_RF95_PA_DAC_DISABLE = 0x04,
    RH_RF95_REG_09_PA_CONFIG = 0x09,
    RH_RF95_PA_SELECT = 0x80,
    RH_RF95_REG_20_PREAMBLE_MSB = 0x20,
    RH_RF95_REG_21_PREAMBLE_LSB = 0x21,
    RH_RF95_MODE_STDBY = 0x01,
    RH_RF95_MODE_RXCONTINUOUS = 0x05,
    RH_RF95_REG_40_DIO_MAPPING1 = 0x40
    ;

let
    _bufLen,
    _rxBufValid,
    _mode,
    RHModeIdle,
    _buf,
    RHModeTx
    ;

function setFrequency(freq) {
    let frf = new Uint32Array(freq * 1000000.0);
    write(RH_RF95_REG_06_FRF_MSB, (frf >> 16) & 0xff);
    write(RH_RF95_REG_07_FRF_MID, (frf >> 8) & 0xff);
    write(RH_RF95_REG_08_FRF_LSB, frf & 0xff);

    return true;
}

function recv(buf, len) {
    if (!available()) {
        return false;
    }
    if (buf && len) {
        if (len > _bufLen - RH_RF95_HEADER_LEN) {
            len = _bufLen - RH_RF95_HEADER_LEN;
            // memcpy(buf, _buf+RH_RF95_HEADER_LEN, *len);
        }
    }
    clearRxBuf();
    return true;
}

function clearRxBuf() {
    _rxBufValid = false;
    _bufLen = 0;
}

function init() {
    write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);
    delay(10);

    if (read(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE)) {
        console.log(parseInt(read(RH_RF95_REG_01_OP_MODE), 16)); // HEX in Dezimal umwandeln
        return false;
    }

    write(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    write(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    setModeIdle();

    // setModemConfig(Bw125Cr45Sf128); // Radio default

    setPreambleLength(8);

    setFrequency(434.0);

    setTxPower(13);

    return true;
}

function setModeIdle() {
    if (_mode != RHModeIdle) {
        write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY);
        _mode = RHModeIdle;
    }
}

function setPreambleLength(bytes) { // uint_16
    write(RH_RF95_REG_20_PREAMBLE_MSB, bytes >> 8);
    write(RH_RF95_REG_21_PREAMBLE_LSB, bytes & 0xff);
}

function setTxPower(power) {
    if (power > 23) power = 23;
    if (power < 5) power = 5;

    if (power > 20) {
        write(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_ENABLE);
        power -= 3;
    }
    else {
        write(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_DISABLE);
    }

    write(RH_RF95_REG_09_PA_CONFIG, RH_RF95_PA_SELECT | (power - 5));
}

function setModeRx() {
    if (_mode != RHModeRx) {
        write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_RXCONTINUOUS);
        write(RH_RF95_REG_40_DIO_MAPPING1, 0x00); // Interrupt on RxDone
        _mode = RHModeRx;
    }
}

function available() {
    if (uartAvailable()) {
        if (uartRead() == 'I') {
            handleInterrupt();
        }
    }

    if (_mode == RHModeTx) return false;
    setModeRx();

    return _rxBufValid;
}