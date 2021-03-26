const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const port = new SerialPort('/dev/ttyUSB0', { baudRate: 115200 });

const parser = new Readline();
port.pipe(parser);

parser.on('data', line => console.log(`> ${line}`));
//port.write('ROBOT POWER ON\n');