'use strict';

const dotenv = require('dotenv')
const net = require('node:net');
const { networkInterfaces } = require('os');
const fs = require('fs');
const { parseModbusPacket, getPacketToSend } = require('./modbusPacket');


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
                
                parseModbusPacket(parsedData.payload.data);
            } catch(e) {
                console.error("parsing error", e);
            }
        })

        let intId = setInterval(() => {
            console.log(" ");
            let idBuff = Buffer.from([0xcc, 0x00]);
            // let messageBuff = Buffer.from("kaka");
            let messageBuff = getPacketToSend();
            let buf = Buffer.concat([idBuff, messageBuff]);
            socket.write(buf)
            console.log("send data: ", buf);
            console.log(" ");
        }, 3000)
        
        socket.on("end", (data) => {
            console.log("\nsocket On end")
            clearInterval(intId);
        });
        
        socket.on("error", (e) => {
            console.log("\nsocket On error:", e)
            clearInterval(intId);
        });

        socket.on("close", (e) => {
            console.log("\nsocket On close:", e)
            clearInterval(intId);
        });
        // socket.end('\ngoodbye\n');
    }).on('error', (err) => {
        // Handle errors here.
        // throw err;
        console.error("createServer Error", err);
    });

    server.on("connection", (e) => {
        console.log("on connection !!!");
    });
    server.on("drop", (e) => {
        console.log("on drop !!!");
    });
    
    // const controller = new AbortController();
    server.on('error', (e) => {
        console.log("\n server On error:", e)
        // if (e.code === 'EADDRINUSE') {
        //     console.error('Address in use, retrying...');
        //     setTimeout(() => {
        //         server.close();
        //         server.listen(PORT, HOST);
        //     }, 1000);
        // }
    });

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
