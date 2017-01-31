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


/* Libraries */
#include <Wire.h>
#include "navigation_controller_types.h"
#include <Servo.h>

/* Variables */

// Received command handling
int lastCommand = 0;
String lastCoord = "";

// Actuator command handling
int actuationCmd = 0;
int lastActuationCmd = 100;

// 'Outdoor - Auto'
int initialLoop = 0;
int currentDistances[4];
const float pi = 3.14159;

// I/O configuration
const int radarTrigPin = 10;
const int radarEchoPin = 11;

// Radar
Servo radarServo;
int radar_angle = 0;
int actual_radar_angle;

/* Core system functions */

/**
 * Sets up the Arduino on boot.
 */
void setup()
{
  // I2C
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  // Serial
  Serial.begin(9600);           // start serial for output
  // Radar
  pinMode(radarTrigPin, OUTPUT); // trig pin as output
  pinMode(radarEchoPin, INPUT); // echo pin as input
  radarServo.attach(12);
  radarServo.write(90);

  Serial.print("* * * READY * * * \n");
}

/**
 * This function will run continuously when no other function is running.
 */
void loop()
{
  delay(100);
  if(actuationCmd != lastActuationCmd){
    Wire.beginTransmission(3);
    Wire.write(actuationCmd);
    Wire.endTransmission();
    lastActuationCmd = 0;
    actuationCmd = 0;
  }
}

/* I2C cimmunication handlers */

/**
 * Handles data received from the master.
 */
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
        actuationCmd = number;
      }
      if(number == 2){
        Serial.print("POWER OFF \n");
        lastCommand = number;
        actuationCmd = number;
      }
      if(number == 3){
        Serial.print("COORDINATE RECEIVED \n");
        lastCommand = number;
        actuationCmd = number;
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

    Serial.print(lastCoord);
    Serial.print("\n");

    lastCommand = 3;
  }
}

/**
 * Responds to request interrupts from the master.
 */
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

/* Outdoor - Auto functions */

/**
 * This is the mode controller for the Outdoor - Auto mode.
 */
void outdoorAutoController(){
  double destinationHeading, currentHeading;
  coord currentPosition, destinationPosition;
  
  currentPosition = getCurrentPosition();
  destinationHeading = getDestinationHeading(&currentPosition, &destinationPosition);
  currentHeading = getCurrentHeading();
  calcMoveToHeading(destinationHeading, currentHeading);
}

coord getCurrentPosition(){
  coord currentPosition;

  return currentPosition;
}

/**
 * Retreives the vehicles destination heading, based on the current coordinate position of the vehicle. 
 */
double getDestinationHeading(coord* currentPosition, coord* destinationPosition){
  int moveEastWest = 0;
  int moveNorthSouth = 0;
  double destinationHeading = 0;
  double currentHeading = 0;

  double a = destinationPosition->lat * pi / 180;
  double b = destinationPosition->lon * pi / 180;
  double c = currentPosition->lat * pi / 180;
  double d = currentPosition->lon * pi / 180;

  if (cos(c) * sin(d - b) == 0){
    if (c > a){
      destinationHeading = 0;   
    }
    else{
      destinationHeading = 180;
    }
  }
  else{
    double angle = atan2(cos(c) * sin(d - b), sin(c) * cos(a) - sin(a) * cos(c) * cos(d - b));
    double exp1 = (angle * 180 / pi + 360);
    double exp2 = 360;
    destinationHeading =  fmod(exp1, exp2);
  }

  Serial.print("Moving in direction: ");
  Serial.println(destinationHeading, DEC);

  return destinationHeading;
}

/**
 * Retreives the vehicles current heading from the IMU compass. 
 */
double getCurrentHeading(){
  return 0;
}

/**
 * Determine the number of degrees to turn and in which direction.
 */
void calcMoveToHeading(double destinationHeading, double currentHeading){
  int degDifference = destinationHeading - currentHeading;
  int degTurn;
  boolean turnLeftNotRight = false;

  if(-180 < degDifference && degDifference <= 180){
    degTurn = degDifference;
  }
  if(degDifference > 180){
    degTurn = degDifference - 360;
  }
  if(degDifference <= -180){
    degTurn = degDifference + 360;
  }
 
  if(degTurn > 0){
    turnLeftNotRight = true;
  }
  if(degTurn < 0){
    turnLeftNotRight = false;
  }

  if(turnLeftNotRight == false){
    //turnRight(degTurn);
  }
  if(turnLeftNotRight == true){
    //turnLeft(degTurn);
  }

  return;
}

/* Radar */

/**
 * Completes one complete 180* radar scan and saves the distances to an array.
 */
 void radarScan(){
  long duration;
  long distance;
    
  for (radar_angle = 0; radar_angle <= 180; radar_angle = radar_angle + 10){ 
    radarServo.write(radar_angle);   
    digitalWrite(radarTrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(radarTrigPin, HIGH);
    delayMicroseconds(10); // 10us
    digitalWrite(radarTrigPin, LOW);
    duration = pulseIn(radarEchoPin, HIGH);
    distance = microsecondsToCentimeters(duration);
    if(distance<200){
      Serial.print("Object Detected at an angle = ");
      actual_radar_angle = radarServo.read();
      Serial.print(actual_radar_angle);
      Serial.print(" deg and distance = ");
      Serial.print(distance);
      Serial.print(" cm");
      Serial.println();
    }
    delay(500);
  }  

  for (radar_angle = 180; radar_angle >=0; radar_angle = radar_angle - 10){ 
    radarServo.write(radar_angle);    
    digitalWrite(radarTrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(radarTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(radarTrigPin, LOW);
    duration = pulseIn(radarEchoPin, HIGH);   
    distance = microsecondsToCentimeters(duration);
    if(distance < 200){
         Serial.print("Object Detected at an angle = ");
         actual_radar_angle = radarServo.read();
         Serial.print(actual_radar_angle);
         Serial.print(" deg and distance = ");
         Serial.print(distance);
         Serial.print(" cm");
         Serial.println();
    }
    delay(500);
  } 
 }

/**
 * Helper function to convert time into distance travelled.
 */
long microsecondsToCentimeters(long us){
  return us / 58;
}

