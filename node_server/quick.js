const { crc16, getPacketToSend } = require("./modbusPacket");



// NumberToUint8Array = number => {
//     var array = [], bigint = BigInt(number)
//     for (let i = 0; i < Math.ceil(Math.floor(Math.log2(new Number(number)) + 1) / 8); i++)
//         array.unshift(new Number((bigint >> BigInt(8 * i)) & 255n))
//     return new Uint8Array(array)
// }
// // const data = Buffer.from([0x01, 0x01, 0x02]);
// // const data = Buffer.from([0xff, 0x03, 0x00, 0x0b, 0x00, 0x01]);
// const data = Buffer.from([0xff, 0x03, 0x00, 0x0b, 0x00, 0x01]); //FF03 00 0B 00 01 E016


// function getCRCBuffer(data) {
//     const crcValue = crc16(data)
//     const array = NumberToUint8Array(crcValue);
//     const arrayBuff = Buffer.from(array);

//     return arrayBuff;
// }

// const crc = getCRCBuffer(data);

// console.log("data", data);
console.log("getPacketToSend", getPacketToSend());