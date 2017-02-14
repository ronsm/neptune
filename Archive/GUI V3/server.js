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
        case '.otf':
            contentType = 'text/otf';
            break;
        case '.eot':
            contentType = 'text/eot';
            break;
        case '.svg':
            contentType = 'text/svg';
            break;
        case '.woff':
            contentType = 'text/woff';
            break;
        case '.ttf':
            contentType = 'text/ttf';
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
                response.end('CONTACT ADMIN '+error.code+' ..\n');
                response.end(); 
            }
        }
        else {
            response.writeHead(200, { 'Content-Type': contentType });
            response.end(content, 'utf-8');
        }
    });

})

server.listen(8000);
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

	// POWER MANAGEMENT
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
    
    // MODE SELECTION
    socket.on('modeSelAuto', function(command){
		console.log(command);
		device1.writeByte(10, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });

    socket.on('modeSelManual', function(command){
		console.log(command);
		device1.writeByte(11, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });
    
    socket.on('modeSelIndoor', function(command){
		console.log(command);
		device1.writeByte(12, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });

    socket.on('modeSelOutdoor', function(command){
		console.log(command);
		device1.writeByte(13, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });
    
    // MANUAL CONTROLS
    
    socket.on('navManForward', function(command){
		console.log(command);
		device1.writeByte(100, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });
    
    socket.on('navManRudder', function(command){
		var commandInt = command.toString();
		
		var commandAdjusted;
		
		if(commandInt == 0){
			commandAdjusted = 101;
		}
		if(commandInt == 1){
			commandAdjusted = 102;
		}
		if(commandInt == 2){
			commandAdjusted = 103;
		}
		if(commandInt == 3){
			commandAdjusted = 104;
		}
		if(commandInt == 4){
			commandAdjusted = 105;
		}
		if(commandInt == 5){
			commandAdjusted = 106;
		}
		if(commandInt == 6){
			commandAdjusted = 107;
		}
		if(commandInt == 7){
			commandAdjusted = 108;
		}
		if(commandInt == 8){
			commandAdjusted = 109;
		}
		if(commandInt == 9){
			commandAdjusted = 110;
		}
		if(commandInt == 10){
			commandAdjusted = 111;
		}
		else{
			commandAdjusted = 112;
		}
		
		console.log(command);
		console.log('rudder position:');
		console.log(commandAdjusted);
		
		device1.writeBytes('', commandAdjusted, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });
    
    socket.on('navManSpeed', function(command){
		var commandInt = command.toString();
		
		var commandAdjusted;
		
		if(commandInt == 0){
			commandAdjusted = 120;
		}
		if(commandInt == 1){
			commandAdjusted = 121;
		}
		if(commandInt == 2){
			commandAdjusted = 122;
		}
		if(commandInt == 3){
			commandAdjusted = 123;
		}
		if(commandInt == 4){
			commandAdjusted = 124;
		}
		if(commandInt == 5){
			commandAdjusted = 125;
		}
		if(commandInt == 6){
			commandAdjusted = 126;
		}
		if(commandInt == 7){
			commandAdjusted = 127;
		}
		if(commandInt == 8){
			commandAdjusted = 128;
		}
		if(commandInt == 9){
			commandAdjusted = 129;
		}
		else{
			commandAdjusted = 130;
		}
		
		console.log(command);
		console.log('rudder position:');
		console.log(commandAdjusted);
		
		device1.writeBytes('', commandAdjusted, function(err) { console.log("error"); console.log(err); });

		newCommand = 1;
    });
    
    // MAP MANAGEMENT
    
    socket.on('submitCoordCmd', function(command){
		var commandString = command.toString();
		console.log(commandString);

		var commandStringSplit = command.split(',');
		var commandStringLeft = commandStringSplit[0].toString();
		var commandStringRight = commandStringSplit[1].toString();
		
		commandStringLeft = commandStringLeft.substr(1);
		commandStringRight = commandStringRight.substr(1);
		commandStringRightFinal = commandStringRight.substr(0, commandStringRight.length - 1);
		
		console.log(commandStringLeft);
		console.log(commandStringRightFinal);
		
		var coordLength = 10;
		var commandStringLeftShort = commandStringLeft.substring(0, coordLength);
		var commandStringRightShort = commandStringRightFinal.substring(0, coordLength);
		
		var commandStringCombined = commandStringLeftShort.concat(","+commandStringRightShort);
		
		commandChars = commandStringCombined.split('');
		console.log(commandChars);
		
		device1.writeBytes('CHANGE_ME', commandChars, function(err) { console.log("error"); console.log(err); });

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
