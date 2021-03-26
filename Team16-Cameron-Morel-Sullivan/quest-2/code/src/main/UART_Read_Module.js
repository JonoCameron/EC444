/*
  Node.js module to read data from the ESP32 over UART. Code adapted from the 
  serialport documentation.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Oct. 2020
*/

const SerialPort = require('serialport');                 // Load the serialport module
const Readline = require('@serialport/parser-readline');  // Load the parser module
const fs = require('fs')                                  // Load the fs module
const port = new SerialPort('/dev/ttyUSB0', {baudRate: 115200});  // Set the target serial port (Linux - RPi)
//const port = new SerialPort('COM3', {baudRate: 115200});   // Set the target serial port (Windows)
//const port = new SerialPort('/dev/cu.SLAB_USBtoUART', {baudRate: 115200});  // Set the target serial port

const parser = new Readline('\n');  // Set the newline character as the delimiter
port.pipe(parser);                  // Pipe the port's data to be delimited by '\n'


/* Write data to sensor_data.csv */
parser.on('data', (data) => {
  let csv_data = data.split(',')
  var filepath = '../res/sensor_data.csv'

  if(csv_data.length != 7){
    console.error('Incomplete Sensor Data Set')
    return;
  }

  let datastr = ''+csv_data[0]+','+csv_data[2]+','+csv_data[4]+','+csv_data[6] +'\n'

  console.log(datastr);

  if(fs.existsSync(filepath)){
    fs.appendFile(filepath, datastr, (err) => {
      console.error(err)
    })
  } else {
    fs.writeFile(filepath, datastr, (err) => {
      console.error(err)
    })
  }
  
});
 /* reset file every minute */
setInterval(()=>{
  fs.writeFileSync('../res/sensor_data.csv', '');
}, 60000)
