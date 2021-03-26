/*
  Creates an express server for handling client requests and MongoDB queries as well as a UDP server to handle fob requests

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
// Server objects
var express = require('express');                  // Load module for express functionality
var server = express();                            // Set server variable to use express
var path = require('path');                        // Load module that handles setting path for express
var MongoClient = require('mongodb').MongoClient;  // Load module that creates a MongoClient object
var mongoFun = require('./mongodb-functions');         // Load custom module that specifies MongoDB functions

// Global variables
var url_db = "mongodb+srv://team16:0H5vJB7WCjh3bDzS@cluster0.b6ubs.gcp.mongodb.net/votes?retryWrites=true&w=majority";  // URL for the MongoDB server
var results = "";                           // Contains results from a MongoDB query
const candidate_list = ['joe biden', 'donald trump', 'test tester']    // hard coded values for the possible candidates
const char_candidate = {
  B : 0,
  R : 1,
  G : 2
}

// UDP Server setup
const dgram = require('dgram'),
udp_server = dgram.createSocket('udp4'),
axios = require('axios')

const web_url = 'http://ghostcamel.ddnss.org:3143/'

var PORT = 6969
var HOST = '192.168.1.16'

// initialize UDP server
udp_server.on('listening', () => {
  console.log('UDP Server Listening...\nPORT: ' + udp_server.address().port + '\nADDR: ' + udp_server.address().address)
})

// check for messages on UDP server port
udp_server.on('message', (msg, rinfo) => {
  console.log('Remote connection from : ' + rinfo.address + ':' + rinfo.port +'\n')
  console.log(' ---> ' + msg + '\n')

  console.log(msg.length)

  if(msg[0] == 86 && msg.length >= 3){ 
    console.log('Fob ID: ' + Number(msg[1]))
    console.log('Candidate ID: ' + String.fromCharCode(msg[2]))
    axios.post(web_url + 'vote', {
      fob_id: Number(msg[1]),
      vote: candidate_list[char_candidate[String.fromCharCode(msg[2])]],
      time: Date.parse(new Date())
    }).then((res, err) => {
      if(err) console.log(err)
      else {
        console.log('Responding to fob')
        if(res.data == 'R'){
          udp_server.send('R' + String.fromCharCode(msg[1]), rinfo.port, rinfo.address, (error)=>{
            if(error){
              console.log(error)
            } else {
              console.log('Sent vote recieved')
            }
          })
        } else {
          udp_server.send('A', rinfo.port, rinfo.address, (error)=>{
            if(error){
              console.log(error)
            } else {
              console.log('Sent affirmative');
            }
          })
        }
      }
    })
  }
})
// HTTP Server setup

// Use URL encoding for handling POST data
server.use(express.urlencoded({extended: true}))
server.use(express.json());


// Handle GET requests to the server's base directory
server.get("/", function (request, response) {
  results = "";  // Reset the results variable
  response.sendFile("candidate_disp.html", {root: path.join(__dirname, "../res")});
});


// Handle JQuery source
server.get("/jquery", function (request, response) {
  response.sendFile("jquery-3.5.1.min.js", {root: path.join(__dirname, "../library")});
});

server.get("/canvas", function (request, response) {
  response.sendFile("canvasjs.min.js", {root: path.join(__dirname, "../library")});
});

// Handle GET requests to MongoDB query results
server.get("/result", function (request, response) {
  response.send(results);
});

server.post('/reset', (req, res) => {
  mongoFun.resetDB((resp) => {
    res.sendFile("candidate_disp.html", {root: path.join(__dirname, "../res")})
  })
})

server.post('/vote', (req, res) => {
  var data = req.body
  console.log(data)
  data.vote = data.vote.toLowerCase()
  mongoFun.postVote(data, (resp) => {
    res.send(resp)
  })
})
// Handle POST requests to the server's query directory
server.post("/query", function (request, response) {
  // Get the user's search input from the POST request
  var candidate = request.body.search;
  console.log('candidate : ' + candidate)
  candidate = candidate.toLowerCase();
  // Only query if the appropriate candidate name is passed
  if (candidate_list.includes(candidate))
  {
    mongoFun.getCandidate(candidate, function(res) {
      console.log("Query returned: " + Object.keys(res).length + " elements");
      results = res;
    });
  }
  else
  {
    console.log("Error: Invalid Candidate name (" + request.body.search + ")");
    results = "";
  }
  response.sendFile("candidate_disp.html", {root: path.join(__dirname, "../res")});
});

server.get('/all', (req, resp) => {
  console.log('Initializing results')
  mongoFun.getAll((res) => {
    console.log(res)
    resp.send(res)
  })
//server.listen(8080);
server.listen(8080, () => {mongoFun.configDB(url_db)})
udp_server.bind(PORT, HOST)
