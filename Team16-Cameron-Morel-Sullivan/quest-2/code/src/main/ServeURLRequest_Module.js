/*
  Node.js module that creates a server to handle GET requests. Specifically 
  functions to serve HTML files, which display CanvasJS. Requires specifying 
  the requested file's subdirectory within src/.

  URL to display sensor data CanvasJS Graph:
  http://localhost:8080/res/sensor_data.html

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Oct. 2020
*/
var http = require('http');  // Load module that creates a server
var url = require('url');    // Load module that splits a web address into readable parts
var fs = require('fs');      // Load module that handles a file system

// Create a server to display a requested graph
http.createServer(function (req, res)
{
  // Parse the URL
  var q = url.parse(req.url, true);
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
}).listen(8080);
