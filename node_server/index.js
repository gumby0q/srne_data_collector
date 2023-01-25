'use strict';

const dotenv = require('dotenv')
const net = require('node:net');
const { networkInterfaces } = require('os');

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

