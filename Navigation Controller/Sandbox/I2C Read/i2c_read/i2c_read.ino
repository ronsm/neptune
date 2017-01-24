// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this
// Created 29 March 2006
// This example code is in the public domain.
#include <Wire.h>

int lastCommand = 0;
String lastCoord = "";

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  Serial.begin(9600);           // start serial for output

  Serial.print("* * * READY * * * \n");
}

void loop()
{
  delay(100);
}

void receiveEvent(int byteCount){

  int number = 0;
  int state = 0;

  //Serial.print(byteCount);

  if(byteCount == 1){
    while(Wire.available()) {
      number = Wire.read();
      
      if(number == 1){
        Serial.print("POWER ON \n");
        lastCommand = number;
      }
      if(number == 2){
        Serial.print("POWER OFF \n");
        lastCommand = number;
      }
      if(number == 3){
        Serial.print("COORDINATE RECEIVED \n");
        lastCommand = number;
      }
    }
  }
  if(byteCount > 20){
    while (1 < Wire.available()) { // loop through all but the last
      int c = Wire.read(); // receive byte as a character
      lastCoord += c;
      //Serial.print(c);         // print the character
    }
    int x = Wire.read();    // receive byte as an integer
    lastCoord += x;

    if(lastCoord

    Serial.print(lastCoord);
    Serial.print("\n");

    lastCommand = 3;
  }
}

void requestEvent(){
  switch (lastCommand) {
    case 1:
      Wire.write("Starting up.... ");
      break;
    case 2:
      Wire.write("Shutting down...");
      break;
    case 3:
      Wire.write("Coord. received!");
      break;
    default: 
      Serial.print(lastCommand);
      Serial.print("\n");
      Wire.write("READY           ");
    break;
  }
}
