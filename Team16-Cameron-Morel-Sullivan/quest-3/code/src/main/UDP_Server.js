/*
  Creates a Node.js UDP server. Code adapted from EC444 Socket Design Pattern 
  brief.

  Node.js module that creates a server to handle GET requests. Specifically 
  functions to serve HTML files, which display CanvasJS. Requires specifying 
  the requested file's subdirectory within src/.

  URL to display sensor data CanvasJS Graph:
  http://<your web address:the port it's set up on>/res/sensor_data.html

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Oct. 2020
*/

var http = require('http');  // Load module that creates a server
var url = require('url');    // Load module that splits a web address into readable parts

// Required module
var dgram = require('dgram');

// Port and IP
var PORT = 3333;
var HOST = '192.168.1.16';  // IP Address of the PI

// Create socket
var server = dgram.createSocket('udp4');

// Variable to hold LED intensity (0-9)
var intensity = 0;

// Write received data to file
const fs = require('fs');

// Create UDP server
server.on('listening', function () {
  var address = server.address();
  console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

fs.writeFile('../res/sensor_data.csv', '', function(err){
  if(err) throw err;
  console.log('Cleared sensor_data.csv!');
})

// On connection, print out received message
server.on('message', function (message, remote) {
  console.log(remote.address + ':' + remote.port +' - ' + message);

  var csv_data = message.toString().split(',');
  var str0 = csv_data[0];
  var newStr0 = str0.slice(1,9);
  var str1 = csv_data[1];
  var newStr1 = str1.slice(1,9);
  var str2 = csv_data[2];
  var newStr2 = str2.slice(1,9);
  var str3 = csv_data[3];
  var newStr3 = str3.slice(1,9);
  var str4 = csv_data[4];
  var newStr4 = str4.slice(1,9);
  var str5 = csv_data[5];
  var newStr5 = str5.slice(1,9);
  var str6 = csv_data[6];
  var newStr6 = str6.slice(1,9);
  var datastr = ''+newStr0+','+newStr1+','+newStr2+','+newStr3+','+newStr4+','+newStr5+','+newStr6+ '\n'

  fs.appendFile('../res/sensor_data.csv', datastr, function(err){
    if(err) throw err;
    console.log('Saved!');
  });

  // Send Ok acknowledgement
  server.send("OK: Set LED " + intensity,remote.port,remote.address,function(error){
    if (error)
    {
      console.log('MEH!');
    }
    else
    {
      console.log('Sent: Ok!');
    }
  });

});

const updateIntensity = (data) => {
  intensity = JSON.parse(data).intensity;
}

const routes = {
  "/postPWMVal" : updateIntensity
}
// Create a server to display a requested graph
http.createServer(function (req, res)
{
  // Parse the URL
  var q = url.parse(req.url, true);
  // Check if the route corresponds to a specific server function
  if(q.pathname in routes){
    let data;
    // receive and parse data from POST
    req.on('data', chunk => {
      data += chunk;
    });
    req.on('end', () => {
      // Pass data to relevant function as JSON object
      data = JSON.stringify(data)
      data = data.replace("undefined", "")
      routes[q.pathname](JSON.parse(data))  // Gives an "unexpected token u in JSON at position 0" error because of the undefined part
      res.writeHead(200, {'Content-Type' : 'text/html'})
      res.write('Success')
      res.end()
    })
  } else {
    var filename = "../" + q.pathname;  // Get out of the main directory (will be in src)

    // Attempt to read the request filename that was passed in the URL
    fs.readFile(filename, function (err, data)
    {
      if (err)
      {
        // Unable to read the file (doesn't exist), so send an error response
        res.writeHead(404, {'Content-Type': 'text/html'});
        return res.end("404: No page for you!");
      }
      else
      {
        // The file exists in the system
        res.writeHead(200, {'Content-Type': 'text/html'});
        res.write(data);
        return res.end();
      }
    });
  }
}).listen(6969); // Change to your port, the one that you router forwards to

// Bind server to port and IP
server.bind(PORT, HOST);

