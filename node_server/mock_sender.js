const fs = require('fs');
const net = require('node:net');


const PORT = 3333;



function main() {
    const client = net.createConnection({ port: PORT }, () => {

        const data = fs.readFileSync('./rs485_dump_01', 'utf8');
        const parsedData = data.split("\n");
    
        const maxIndex = parsedData.length;
        let index = 0;
        setInterval(() => {
            const row = parsedData[index];
            // console.log(parsedData);
            index++;
            if (index > maxIndex) {
                index = 0;
            }
    
            const jsonStr = `{"uuid": "0cb815f5eb0c","type": "RS485_SNIFF","payload": {"data": "${row}"}}`;
            client.write(jsonStr);
            console.log("send", row);
        }, 2000);
    
        // 'connect' listener.
        console.log('connected to server!');
    });
    // client.on('data', (data) => {
    //     console.log(data.toString());
    //     client.end();
    // });
    client.on('end', () => {
        setTimeout(() => {
            main();
        }, 5000);
        console.log('disconnected from server');
    });
}


main();
