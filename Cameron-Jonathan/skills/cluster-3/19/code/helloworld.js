var http = require('http');

http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Hello World');
}).listen(3001, "192.168.1.114");
console.log('Server running at http://192.168.1.114:3001/');