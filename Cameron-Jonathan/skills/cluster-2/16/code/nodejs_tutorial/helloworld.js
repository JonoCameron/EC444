var http = require('http');

http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Hello World');
}).listen(3001, "127.0.0.1");
console.log('Server running at http://127.0.0.1:3001/');