/*
  Creates an express server for handling client requests and a UDP client to control the car.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
// Server objects
var express = require('express');                  // Load module for express functionality
var server = express();                            // Set server variable to use express
var path = require('path');                        // Load module that handles setting path for express

// UDP Server setup
const dgram = require('dgram'),
udp_client = dgram.createSocket('udp4'),
axios = require('axios')

//const web_url = 'http://localhost:8080/'

// Details on the car's UDP server
var PORT = 2222
// var ADDR = '192.168.1.241'
var ADDR = '192.168.1.22'


// Use URL encoding for handling POST data
server.use(express.urlencoded({extended: true}))
server.use(express.json());


// Handle GET requests to the server's base directory
server.get("/", function (request, response) {
  response.sendFile("control_car.html", {root: path.join(__dirname, "../res")});
});


// Handle JQuery source
server.get("/jquery", function (request, response) {
  response.sendFile("jquery-3.5.1.min.js", {root: path.join(__dirname, "../library")});
});


// Sends a UDP packet to toggle the car
server.post('/toggle', (request, response) => {
  udp_client.send('!', 0, 1, PORT, ADDR, function(error, bytes){
    if (error)
    {
      console.log(error)
    }
    else
    {
      console.log('Successfully sent command to car')
    }
  })
  response.sendFile("control_car.html", {root: path.join(__dirname, "../res")});
})


server.listen(8080);
