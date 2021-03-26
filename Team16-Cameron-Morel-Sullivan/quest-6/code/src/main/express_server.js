/*
  Creates an express server for handling client requests and a UDP client to control the dish ESP.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
// Server objects
var express = require('express');                  // Load module for express functionality
var server = express();                            // Set server variable to use express
var path = require('path');                        // Load module that handles setting path for express
var sensor_active = false

// UDP Server setup
const dgram = require('dgram'),
udp_client = dgram.createSocket('udp4')

// Serial Port Setup
//const serialpath = '/dev/tty.usbserial-017499C3',
//const serialpath = 'COM4',
const serialpath = '/dev/ttyUSB0'
serial = require('serialport'),
port = new serial(serialpath, {baudRate: 115200},  (err) => {
  console.error(err)                              // if there is an error print it to console
}),
ReadLine = serial.parsers.Readline,
parser = new ReadLine()
port.pipe(parser)

var sensor_data_arr = []

port.on('open', () => {
  console.log("OPEN\nBAUD : " + port.baudRate)
})

parser.on('data', (data) => {
  let raw_data = data.toString()
  var sensor_data = {
    't': 0,
    'A': 0,
    'T': 0,
    'D': 0,
    'X': 0,
    'Y': 0,
    'Z': 0,
    'R': 0,
    'P': 0,
    's': 0
  }
  if(raw_data.length == 99 && raw_data[0] === "t"){
    console.log(raw_data);
    for(prop in sensor_data) {
      sensor_data[prop] = parseFloat(raw_data.slice(raw_data.indexOf(prop)+1, raw_data.indexOf(prop)+8))
      //console.log(sensor_data[prop]);
      //console.log(sensor_data.t + ',' + sensor_data.A + ',' + sensor_data.T + ',' + sensor_data.D + ',' + sensor_data.X + ',' + sensor_data.s);
    }

    // Check if we completed a full cycle and need to reset the array
    if ( sensor_data_arr.length >= (180 * 2 / 5) )
    {
      sensor_data_arr = [];
    }

    sensor_data_arr = [...sensor_data_arr , sensor_data];
  }
})


//const web_url = 'http://localhost:8080/'

// Details on the UDP server
var PORT = 6969
var ADDR = '192.168.1.241'

// Use URL encoding for handling POST data
server.use(express.urlencoded({extended: true}))
server.use(express.json());


// Handle GET requests to the server's base directory
server.get("/", function (request, response) {
  response.sendFile("radar.html", {root: path.join(__dirname, "../res")});
});


// Handle JQuery source
server.get("/jquery", function (request, response) {
  response.sendFile("jquery-3.5.1.min.js", {root: path.join(__dirname, "../library")});
});


// Sends a UDP packet to toggle the radar dish ON
server.post('/toggleON', (request, response) => {
  udp_client.send('!1', 0, 2, PORT, ADDR, function(error, bytes){
    if (error)
    {
      console.log(error)
    }
    else
    {
      console.log('Successfully told the radar dish to turn ON')
    }
  })
  response.sendFile("radar.html", {root: path.join(__dirname, "../res")});
})


// Sends a UDP packet to toggle the radar dish OFF
server.post('/toggleOFF', (request, response) => {
  udp_client.send('!0', 0, 2, PORT, ADDR, function(error, bytes){
    if (error)
    {
      console.log(error)
    }
    else
    {
      console.log('Successfully told the radar dish to turn OFF')
    }
  })
  response.sendFile("radar.html", {root: path.join(__dirname, "../res")});
})


// Fetches the recorded UART sensor data, the converts the polar coordinates to Cartesian coordinates
server.get("/sensor_data", (req, res) => {
  //console.log('Sensor Data Fetched')
  res.send(sensor_data_arr.map((data_block) => {
    return {
      'x' : (data_block.D/100)*Math.cos((Math.PI * data_block.A)/180),
      'y' : (data_block.D/100)*Math.sin((Math.PI * data_block.A)/180)
    }
  }))
})

// Fetches the recorded UART sensor data, the converts the polar coordinates to Cartesian coordinates
server.get("/refresh_data", (req, res) => {
  //console.log('Data Refreshed')
  res.send(sensor_data_arr[sensor_data_arr.length-1])
})


server.listen(12345);
