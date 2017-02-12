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
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

/* Definitions */
#define GPSECHO true

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
const int radarTrigPin = 22;
const int radarEchoPin = 23;

// Radar
Servo radarServo;
int radar_angle = 0;
int actual_radar_angle;

// Mapping
bool maparray[50][25];    
int prevX[20];                 
int prevY[20];
int pos;
int Xaxis;
int Yaxis;
int radarDistances[18];

// GPS
Adafruit_GPS GPS(&Serial1);
HardwareSerial mySerial = Serial1;
double latitude;
double longitude;
uint32_t timer = millis();
boolean usingInterrupt = false;
void useInterrupt(boolean);

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
  Serial.print("I2C: OK \n");
  
  // Serial
  Serial.begin(57600);           // start serial for output
  Serial.print("Serial: OK \n");
  
  // Radar
  pinMode(radarTrigPin, OUTPUT); // trig pin as output
  pinMode(radarEchoPin, INPUT); // echo pin as input
  radarServo.attach(12);
  radarServo.write(90);
  Serial.print("Radar: OK \n");

  // GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  useInterrupt(true);
  Serial.print("GPS: OK \n");

  // Ready
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

  // Radar
  long rangeDuration;
  long rangeDistance;
  int averageDistance = 0;
  radarScan();

  // GPS
  gpsLoop();
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
    
    pos = radar_angle/10;  
    maparray [prevX[pos]][prevY[pos]] = false;
    
    digitalWrite(radarTrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(radarTrigPin, HIGH);
    delayMicroseconds(10); // 10us
    digitalWrite(radarTrigPin, LOW);
    duration = pulseIn(radarEchoPin, HIGH);
    distance = microsecondsToCentimeters(duration);
    
    Serial.print("Object Detected at an angle = ");

    // Store
    Xaxis = (distance * sin(radar_angle) + 25) / 10;           // get x coordinate according to trigonometry (+50 for placement along the axis)
    Yaxis = (distance * cos(radar_angle)) / 10;                // get y coordinate ''
    maparray [Xaxis][Yaxis] = true;                            // place obstacle in map array
    prevX[pos] = Xaxis;                                        // store coordinates in array according to servo position
    prevY[pos] = Yaxis;

    if(distance < 200){
      radarDistances[(radar_angle/10)] = distance;
      Serial.print(radar_angle);
      Serial.print(" deg and distance = ");
      Serial.print(distance);
      Serial.print(" cm");
      Serial.println();
    }
    else{
      radarDistances[radar_angle/10] = 0;
      Serial.print(radar_angle);
      Serial.print(" deg and distance = ");
      Serial.print("OUT OF RANGE");
      Serial.println();
    }
    delay(200);
  }
  
  radarPrintDistances();
  
  radarCalcDirection();
}

/**
 * Prints the current set of distances from 0-180*.
 */
void radarPrintDistances(){
  Serial.print("\n");
  for(int i = 0; i <= 18; i++){
    Serial.print(radarDistances[i]);
    Serial.print("\t");
  }
  Serial.print("\n");
  return;
}

/**
 * Calculates best direction of travel based on obstacles detected by scanning radar.
 */
void radarCalcDirection(){
  int possibleDirection  = 0;
  int bestPossibleDirection = 0;
  int radarClear = 0;
  int bestRadarClear = 0;
  bool directionFound = false;
  bool clearDirections[18];
  
  for(int i = 0; i <= 18; i++){
    if(radarDistances[i] >= 60){
      clearDirections[i] = true;
    }
    else{
      clearDirections[i] = false;
    }
  }

  for(int i = 0; i <= 16; i++){
    if((clearDirections[i] == true) && (clearDirections[i+1] == true) && (clearDirections[i+2] == true)){
      directionFound = true;
      possibleDirection = ((i+1)*10);
      radarClear = radarDistances[i+1];
      if(radarClear > bestRadarClear){
        bestPossibleDirection = possibleDirection;
        bestRadarClear = radarClear;
      }
    }
  }

  if(directionFound == false){
    bestPossibleDirection = -1;
  }
  
  Serial.print("Best direction to navigate: ");
  Serial.print(bestPossibleDirection);
  Serial.print("\n");
}

/**
 * Helper function to convert time into distance travelled.
 */
long microsecondsToCentimeters(long us){
  return us / 58;
}

/* GPS */

SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void gpsLoop()                     // run over and over again
{
  if (! usingInterrupt) {
    char c = GPS.read();
    if (GPSECHO)
      if (c){
        //Serial.print(c);
      }
  }

  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }

  if (timer > millis())  timer = millis();

  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    Serial.println("Location: ");
    Serial.println("");
    Serial.println(GPSlatitude (), 4);
    Serial.println(GPSlongitude (), 4);
    Serial.println("");
  }
}

double GPSlatitude (){
  if (GPS.fix) {
    return GPS.latitudeDegrees;
  }
  return 0;
}

double GPSlongitude (){
  if (GPS.fix) {
    return GPS.longitudeDegrees;
  }
  return 0;
}
