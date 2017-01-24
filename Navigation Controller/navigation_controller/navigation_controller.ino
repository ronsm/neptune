/**
 * navigation_controller.ino
 *
 * This is the primary source file for the Neptune on-board navigation controller.
 * It is responsible for responding to commands sent from the hovercraft's computer
 * in order to facilitate navigation using the connected sensors and actuators.
 *
 * LICENSE: This source file is subject to a Creative Commons
 * Attribution-NonCommercial 4.0 International (CC BY-NC 4.0) License.
 * Full details of this license are available online at:
 * https://creativecommons.org/licenses/by-nc/4.0/.
 *
 * @package    Neptune
 * @author     Ronnie Smith <ronniesmith@outlook.com>
 * @copyright  2017, Ronnie Smith
 * @license    https://creativecommons.org/licenses/by-nc/4.0/ CC BY-NC 4.0
 * @version    1.0
 * @link       https://github.com/ronsm/neptune
 * 
 * ATTRIBUTIONS: This project uses and derives open source code and packages from
 * various authors, which are attributed here where possible.
 *    1) "Arduino-based Ultrasonic Radar" by Ashutosh Bhatt
 *        http://www.engineersgarage.com/contribution/arduino-based-ultrasonic-radar
 */
 
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
