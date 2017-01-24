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

server.listen(7000);
console.log('Server running at http://127.0.0.1:9000/');

// use socket.io
var io = require('socket.io').listen(server);

//turn off debug
io.set('log level', 1);

// sockets variables
var interprettedMsg;
var newCommand = 0;

// define interactions with client
io.sockets.on('connection', function(socket){

    var device1 = new i2c(0x18, {device: '/dev/i2c-1', debug: false});
    device1.setAddress(0x4);

    //recieve client data
    socket.on('client_data', function(data){
        process.stdout.write(data.letter);
    });

    socket.on('powerOnCmd', function(command){
	console.log(command);
	device1.writeByte(1, function(err) { console.log("error"); console.log(err); });

	newCommand = 1;
    });

    socket.on('powerOffCmd', function(command){
	console.log(command);
	device1.writeByte(2, function(err) { console.log("error"); console.log(err); });

	newCommand = 1;
    });

    socket.on('logUpdate', function(logMsg){
	if(newCommand == 0){
		return;
	}

	var incomingMsg = device1.readBytes('acknowledgeCmd', 16, function(err, res) {});

        interpretMsg(incomingMsg);
        interprettedMsg = interprettedMsg + "\n";
        io.emit('logWrite', interprettedMsg);

	newCommand = 0;
    });

});

function interpretMsg(msg){
	console.log(msg);
	
	var i;
	var asciiMsg = msg.toString('ascii');
	var msgArray = asciiMsg.split(' ');
	for(i = 1; i < msg.length; i++){
		var tmpChar = String.fromCharCode(msgArray[i]);
		process.stdout.write(tmpChar);
	}
	interprettedMsg = msg.toString('utf8');
	console.log(interprettedMsg);
}
