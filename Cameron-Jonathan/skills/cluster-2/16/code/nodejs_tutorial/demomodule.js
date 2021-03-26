var http = require('http');
var dt = require('./firstmodule');

http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Welcome Node.js, the date and time are currently: ' + dt.myDateTime());
}).listen(3001, "127.0.0.1");
console.log('Server running at http://127.0.0.1:3001/');