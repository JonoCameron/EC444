const dgram = require('dgram'),
udp_server = dgram.createSocket('udp4')


//var PORT = 3333           // UDP server port
//var HOST = '10.0.0.217'    // example RaspberryPi address **change accordingly**
var PORT = 3141
var HOST = '98.171.20.181'

udp_server.on('listening', () => {
    console.log('UDP Server Listening...\nPORT: ' + udp_server.address().port + '\nADDR: ' + udp_server.address().address)
})

udp_server.on("message", (msg, rinfo) => {
    console.log(msg)
    if(msg[0] == 82){
        console.log(msg[0])
        console.log(msg[1])
        udp_server.send('R', PORT, HOST, (err) => console.log(err))
        udp_server.close()
    }

    if(msg[0] == 65) {
        console.log(msg[0])
        udp_server.close()
    }
})

udp_server.bind(PORT+5, '192.168.1.6')

udp_server.send('V' + String.fromCharCode(1) + 'B' + '\0', PORT, HOST, (err) => console.log(err))
