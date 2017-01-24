var http = require('http');
var url = require('url');
var fs = require('fs');
var server;
var i2c = require('i2c');

var http = require('http');
var fs = require('fs');
var path = require('path');

server = http.createServer(function (request, response) {
    console.log('request starting...');

    var filePath = '.' + request.url;
    if (filePath == './')
        filePath = './index.html';

    var extname = path.extname(filePath);
    var contentType = 'text/html';
    switch (extname) {
        case '.js':
            contentType = 'text/javascript';
            break;
        case '.css':
            contentType = 'text/css';
            break;
        case '.json':
            contentType = 'application/json';
            break;
        case '.png':
            contentType = 'image/png';
            break;      
        case '.jpg':
            contentType = 'image/jpg';
            break;
        case '.wav':
            contentType = 'audio/wav';
            break;
    }

    fs.readFile(filePath, function(error, content) {
        if (error) {
            if(error.code == 'ENOENT'){
                fs.readFile('./404.html', function(error, content) {
                    response.writeHead(200, { 'Content-Type': contentType });
                    response.end(content, 'utf-8');
                });
            }
            else {
                response.writeHead(500);
                response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
                response.end(); 
            }
        }
        else {
            response.writeHead(200, { 'Content-Type': contentType });
            response.end(content, 'utf-8');
        }
    });

})

server.listen(9000);
console.log('Server running at http://127.0.0.1:9000/');

// use socket.io
var io = require('socket.io').listen(server);

//turn off debug
io.set('log level', 1);

// define interactions with client
io.sockets.on('connection', function(socket){
    //send data to client
    setInterval(function(){
        socket.emit('date', {'date': new Date()});
    }, 1000);

    //recieve client data
    socket.on('client_data', function(data){
        process.stdout.write(data.letter);
    });

    socket.on('powerOnCmd', function(command){
	console.log(command);
	var device1 = new i2c(0x18, {device: '/dev/i2c-1', debug: false});
	device1.setAddress(0x4);
	device1.writeByte(0x1, function(err) { console.log("error"); console.log(err); });
    });

    socket.on('powerOffCmd', function(command){
	console.log(command);
	var device1 = new i2c(0x18, {device: '/dev/i2c-1', debug: false});
	device1.setAddress(0x4);
	device1.writeByte(0x2, function(err) { console.log("error"); console.log(err); });
    });
});

