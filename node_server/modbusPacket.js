



/**
 * Calculates the buffers CRC16.
 *
 * @param {Buffer} buffer the data buffer.
 * @return {number} the calculated CRC16.
 * 
 * Source: github.com/yaacov/node-modbus-serial
 */
function crc16(buffer) {
    var crc = 0xFFFF;
    var odd;

    for (var i = 0; i < buffer.length; i++) {
        crc = crc ^ buffer[i];

        for (var j = 0; j < 8; j++) {
            odd = crc & 0x0001;
            crc = crc >> 1;
            if (odd) {
                crc = crc ^ 0xA001;
            }
        }
    }

    return crc;
};

const NumberToUint8Array = number => {
    var array = [], bigint = BigInt(number)
    for (let i = 0; i < Math.ceil(Math.floor(Math.log2(new Number(number)) + 1) / 8); i++)
        array.unshift(new Number((bigint >> BigInt(8 * i)) & 255n))
    return new Uint8Array(array)
}

function getCRCBuffer(data) {
    const crcValue = crc16(data)
    const array = NumberToUint8Array(crcValue);
    const arrayBuff = Buffer.from(array);

    return arrayBuff;
}

function parsePacketData(data, packetName) {
    // packetName
    const batteryVoltage = data.readInt16BE(0) * 0.1;
    console.log("data BatteryVoltage: ", batteryVoltage);
}

function parseModbusPacket(data) {
    const inputBuffer = Buffer.from(data, "hex");
    console.log("inputBuffer", inputBuffer);
    const bufferSlice = inputBuffer.subarray(0, inputBuffer.length-2);
    const bufferDataSlice = inputBuffer.subarray(3, inputBuffer.length-2);
    const bufferCRCSlice = inputBuffer.subarray(inputBuffer.length-2, inputBuffer.length);
    const crcCalculated = getCRCBuffer(bufferSlice);
    // console.log("bufferSlice", bufferSlice);
    // console.log("bufferCRCSlice", bufferCRCSlice);
    // console.log("crcCalculated", crcCalculated);
    console.log("bufferDataSlice", bufferDataSlice);
    
    const isCRCValid = (crcCalculated[0] == bufferCRCSlice[1]) && (crcCalculated[0] == bufferCRCSlice[1]);
    // console.log("isCRCValid", isCRCValid);

    if (isCRCValid) {
        parsePacketData(bufferDataSlice);
    } else {
        throw Error("CRC is bad");
    }
}


/* example */
// return Buffer.from([0xFF, 0x03, 0x00, 0x0B, 0x00, 0x01, 0xE0, 0x16]) /* default one */

// const masterStart = Buffer.from([0xFF]);
// const devAddress = Buffer.from([0x03]);
// const readAddress = Buffer.from([0x00, 0x0B]);
// const readWords = 1;
// const readLength = Buffer.from([0, readWords]);
// let messageBuffer = Buffer.concat([masterStart, devAddress, readAddress, readLength]);
// const crc = getCRCBuffer(messageBuffer);
// const finalBuffer = Buffer.concat([messageBuffer, Buffer.from([crc[1]]), Buffer.from([crc[0]])]);

function getPacketToSend() {
    // 000CH
    const masterStart = Buffer.from([0xFF]);
    const devAddress = Buffer.from([0x03]);
    const readAddress = Buffer.from([0x01, 0x01]);
    // const readWords = 16/2;
    const readWords = 1;
    const readLength = Buffer.from([0, readWords]);
    let messageBuffer = Buffer.concat([masterStart, devAddress, readAddress, readLength]);
    const crc = getCRCBuffer(messageBuffer);
    const finalBuffer = Buffer.concat([messageBuffer, Buffer.from([crc[1]]), Buffer.from([crc[0]])]);

    return finalBuffer;
}

// {"num":"112","name":"BatteryVoltage","address":"0101","type":"UInt16BE","rate":0.1,"format": 1,"unit":"V"},
module.exports = {
    parseModbusPacket,
    getPacketToSend,
    crc16,
    getCRCBuffer,
};