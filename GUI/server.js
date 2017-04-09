/*
 * server.js
 *
 * This is the Node application responsible for providing communication
 * between the HTML-based GUI and the hardware of the Raspberry Pi, including
 * the I2C and GPIO functionality.
 *
 * @package    Neptune
 * @author     Ronnie Smith <ronniesmith@outlook.com>
 * @version    1.0
 * @link       https://github.com/ronsm/neptune
 *
 * ATTRIBUTIONS: This project uses and derives open source code and packages from
 * various authors, which are attributed here where possible.
 *    1) NPM module example usage:
 *        i2c - https://www.npmjs.com/package/i2c
 *	      serialport - https://www.npmjs.com/package/serialport
 *	      socket.io - https://www.npmjs.com/package/socket.io
 */

var http = require('http');
var url = require('url');
var fs = require('fs');
var i2c = require('i2c');
var fs = require('fs');
var path = require('path');
var SerialPort = require('serialport');

var server;

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
console.log('Server running at http://127.0.0.1:8000/');

// ERROR HANDLER

function errHandler(err){
	if(err != null){
		var errString = '[SERVER] ' + err + '\n';
		io.emit('logWrite', errString);
	}
}

// SERIALPORT

// NOTE: To start this application without an Arduino device connected
// to the bottom-right USB port, you must comment out everything from
// the next line down to the Socket.io section.

'use strict';

const parsers = SerialPort.parsers;

const parser = new parsers.readline({
	delimiter: '\r\n'
});

var port = new SerialPort("/dev/ttyACM0", {
	baudRate: 57600,
	parser: SerialPort.parsers.readline('\n')
});

port.on("open", function(){
	console.log('Opened serial port @ 57600 baud');
});

port.on("data", function(data){
	console.log('' + data);
	var sub = data.substring(1,4);
	if(sub == "NAV"){
		serialMsg = '' + data + '\n';
		io.emit('logWrite', serialMsg);
	}
	if(sub == "RAD"){
		serialMsg = '' + data + '\n';
		io.emit('setRadarData', serialMsg);
	}

	var scString = data.substring(0, 4);
	var scInt = Number(scString);
	if(!isNaN(scInt)){
		checkStatusCode(scInt);
		console.log(scInt);
	}
});

function checkStatusCode(code){
	var element = 100; // No element, no value
	var value = 100;

	if(code == 1000){
		element = 1;
		value = 0;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code == 1001){
		element = 1;
		value = 1;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code == 2000){
		element = 2;
		value = 0;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code == 20001){
		element = 2;
		value = 1;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code >= 3000 && code <= 3010){
		element = 3;
		if(code == 3000) value = 0;
		if(code == 3001) value = 1;
		if(code == 3002) value = 2;
		if(code == 3003) value = 3;
		if(code == 3004) value = 4;
		if(code == 3005) value = 5;
		if(code == 3006) value = 6;
		if(code == 3007) value = 7;
		if(code == 3008) value = 8;
		if(code == 3009) value = 9;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code >= 3990 && code <= 4010){
		element = 4;
		if(code == 4000) value = 0;
		if(code == 4001) value = 1;
		if(code == 4002) value = 2;
		if(code == 4003) value = 3;
		if(code == 4004) value = 4;
		if(code == 4005) value = 5;
		if(code == 4006) value = 6;
		if(code == 4007) value = 7;
		if(code == 4008) value = 8;
		if(code == 4009) value = 9;
		if(code == 4010) value = 10;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code == 5000){
		element = 5;
		value = 0;
		io.emit('interfaceRefresh', element, value);
	}
	else if(code == 5000){
		element = 5;
		value = 1;
		io.emit('interfaceRefresh', element, value);
	}
}

// SOCKET.IO

var io = require('socket.io').listen(server);
io.set('log level', 1);
var interprettedMsg;
var newCommand = 0;

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
		device1.writeByte(1, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('powerOffCmd', function(command){
		console.log(command);
		device1.writeByte(2, function(err) { errHandler(err) });

		newCommand = 1;
    });

    // MODE SELECTION
    socket.on('modeSelAuto', function(command){
		console.log(command);
		device1.writeByte(10, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('modeSelManual', function(command){
		console.log(command);
		device1.writeByte(11, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('modeSelIndoor', function(command){
		console.log(command);
		device1.writeByte(12, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('modeSelOutdoor', function(command){
		console.log(command);
		device1.writeByte(13, function(err) { errHandler(err) });

		newCommand = 1;
    });

    // MANUAL CONTROLS

    socket.on('navManForward', function(command){
		console.log(command);
		device1.writeByte(100, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('navManRudder', function(command){
		var commandInt = command.toString();

		var commandAdjusted;
		commandAdjusted = 112;

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

		console.log(command);
		console.log('rudder position:');
		console.log(commandAdjusted);

		device1.writeByte(commandAdjusted, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('navManSpeed', function(command){
		var commandInt = command.toString();

		var commandAdjusted;
		commandAdjusted = 130;

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

		console.log(command);
		console.log('rudder position:');
		console.log(commandAdjusted);

		device1.writeByte(commandAdjusted, function(err) { errHandler(err) });

		newCommand = 1;
    });

    // MAP MANAGEMENT

    socket.on('submitCoordCmd', function(command){

		device1.writeByte(140, function(err) { errHandler(err) });

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

		device1.writeBytes('COORDINATES', commandChars, function(err) { errHandler(err) });

		newCommand = 1;
    });

    socket.on('submitIndoorStart', function(command) {

		var commandString = command.toString();

		var commandChars = commandString.split('');

		device1.writeByte(141, function(err) { errHandler(err) });

		device1.writeBytes('COORDINATES', commandChars, function(err) { errHandler(err) });

		console.log(commandChars);

		newCommand = 1;
	});

	socket.on('submitIndoorDest', function(command) {

		var commandString = command.toString();

		var commandChars = commandString.split('');

		device1.writeByte(142, function(err) { errHandler(err) });

		device1.writeBytes('COORDINATES', commandChars, function(err) { errHandler(err) });

		console.log(commandChars);

		newCommand = 1;
	});

  // AUTOMATIC LOG UPDATING

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
