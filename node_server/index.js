'use strict';

const dotenv = require('dotenv')
const net = require('node:net');
const { networkInterfaces } = require('os');
const fs = require('fs');


function writeJSONDataToFile(fileName, data) {
    fs.stat(fileName, function (err, stat) {
        const strData = data + "\n";

        if (err == null) {
            // console.log('File exists');

            //write the actual data and end with newline
            // var csv = json2csv(toCsv) + newLine;
            // console.log("csv", csv);
            fs.appendFile(fileName, strData, function (err) {
                if (err) {
                    throw err;
                }
                // console.log('The "data to append" was appended to file!');
            });
        } else {
            //write the headers and newline
            console.log('New file, just writing headers');
            // fields = fields + newLine;
    
            fs.writeFile(fileName, strData, function (err) {
                if (err) {
                    throw err;
                }
                // console.log('file saved');
            });
        }
    });
}


const nets = networkInterfaces();
const results = Object.create(null); // Or just '{}', an empty object

for (const name of Object.keys(nets)) {
    for (const net of nets[name]) {
        // Skip over non-IPv4 and internal (i.e. 127.0.0.1) addresses
        // 'IPv4' is in Node <= 17, from 18 it's a number 4 or 6
        const familyV4Value = typeof net.family === 'string' ? 'IPv4' : 4
        if (net.family === familyV4Value && !net.internal) {
            if (!results[name]) {
                results[name] = [];
            }
            results[name].push(net.address);
        }
    }
}


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


function main() {

    const config = dotenv.config().parsed;
    console.log("config", config)
    console.log("ip", results)


    const server = net.createServer((socket) => {
        socket.on("data", (data) => {
            console.log("data", data);
            try {
                const parsedData = JSON.parse(data.toString());
                // JSON.parse()

                // writeJSONDataToFile("rs485_dump_01", parsedData.payload.data);
                console.log("parsedData", parsedData)
            } catch(e) {
                console.error("parsing error", e);
            }
        })
        socket.end('goodbye\n');
    }).on('error', (err) => {
        // Handle errors here.
        // throw err;
        console.error("createServer Error", err);
    });
    
    // const controller = new AbortController();
    // server.on('error', (e) => {
    //     if (e.code === 'EADDRINUSE') {
    //       console.error('Address in use, retrying...');
    //       setTimeout(() => {
    //         server.close();
    //         server.listen(PORT, HOST);
    //       }, 1000);
    //     }
    //   });

    // Grab an arbitrary unused port.
    server.listen({
        // host: 'localhost',
        port: config.TCP_PORT,
        // signal: controller.signal,
      }, () => {
        const servData = server.address();
        console.log('opened server on', servData);
    });
}

main();

