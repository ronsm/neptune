/**
 * navigation_controller.ino
 *
 * This is the primary source file for the Neptune on-board navigation controller.
 * It is responsible for responding to commands sent from the hovercraft's computer
 * in order to facilitate navigation using the connected sensors and actuators.
 *
 * @package    Neptune
 * @author(s)  Ronnie Smith
 *             Alex James
 * @version    1.0
 * @link       https://github.com/ronsm/neptune
 *
 * NOTES: Below are some important notes regarding usage of this program.
 *    1) To send a message from the Navigation Controller and have it displayed on
 *       the end-user log (on the web GUI), you must preface the string in the
 *       'Serial.print()' command with "[NAV-CON]" and end the string with "\n".
 *       Example: Serial.print("[NAV-CON] The current heading is 234* \n");s
 *
 * ATTRIBUTIONS: This project uses and derives open source code and packages from
 * various authors, which are attributed here where possible.
 *    1) "Arduino-based Ultrasonic Radar" by Ashutosh Bhatt
 *       http://www.engineersgarage.com/contribution/arduino-based-ultrasonic-radar
 *    2) "Bearing between two points on the earth" by Haishi
 *       http://haishibai.blogspot.co.uk/2009/09/bearing-between-two-points-on-earth.html
 *    3) Libraries included below available at ../Libraries/
 */


/* Libraries */
#include <Wire.h>
#include "navigation_controller_types.h"
#include <Servo.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <MPU9250.h>
#include <quaternionFilters.h>
#include <SPI.h>
#include <QueueArray.h>

/* Definitions */
#define GPSECHO false

/* Variables */

// Master settings
bool environmentMode;
bool controlMode;
bool demoMode;

// Manual mode
int rudderPos;
int speedSel;

// Received command handling
QueueArray <int> commandQueue;
int lastCommand = 0;
String lastCoord = "";
double lastCoordLat;
double lastCoordLon;
String lastIndoorStartCoord = "";
String lastIndoorDestCoord = "";
int lastIndoorStartCoordX;
int lastIndoorStartCoordY;
int lastIndoorDestCoordX;
int lastIndoorDestCoordY;

// Actuator command handling
int actuationCmd = 0;
int lastActuationCmd = 100;
bool awaitingResponse = false;

// 'Outdoor - Auto'
int initialLoop = 0;
int currentDistances[4];
const float pi = 3.141593;

// Ultrasonic sensors
const int radarTrigPin = 52;
const int radarEchoPin = 50;

const int distLeftTrig = 48;
const int distLeftEcho = 46;

const int distCenterTrig = 44;
const int distCenterEcho = 42;

const int distRightTrig = 40;
const int distRightEcho = 38;

bool obstacleDirection[3];
bool obstacleCritical = false;

int previousDeg;

// Radar
Servo radarServo;
int radar_angle = 0;
int actual_radar_angle;
int bestPossibleDirection = 0;

// Mapping
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

// IMU
MPU9250 IMU;
double lastKnownHeading;
int mxcalmax = 0;
int mxcalmin = 1000;
int mycalmax = 0;
int mycalmin = 1000;
int MagX;
int MagY;
double prevacctime = 0;
double totaldistance;
double accelvelocity = 0;

/* Core system functions */

/**
 * Sets up the Arduino on boot.
 */
void setup() {
  // Serial
  // -- Baud rates higher than 57600 are less stable on the server end.
  //    This rate is also hard coded into server.js, do not change.
  Serial.begin(57600);           // start serial for output
  Serial.print("Serial: OK \n");

  // I2C
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.print("I2C: OK \n");

  // Radar
  // -- For the rotating sensor on top of the hovercraft.
  pinMode(radarTrigPin, OUTPUT);  // trig pin as output
  pinMode(radarEchoPin, INPUT);   // echo pin as input
  radarServo.attach(12);
  radarServo.write(90);
  Serial.print("Radar: OK \n");

  // Distance sensors
  // -- For the three fixed distance sensors.
  pinMode(distLeftTrig, OUTPUT);
  pinMode(distLeftEcho, INPUT);

  pinMode(distCenterTrig, OUTPUT);
  pinMode(distCenterEcho, INPUT);

  pinMode(distRightTrig, OUTPUT);
  pinMode(distRightEcho, INPUT);

  // GPS
  // -- Sets up the GPS module as described in the datasheet.
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  useInterrupt(true);
  delay(1000);
  mySerial.println(PMTK_Q_RELEASE);
  Serial.print("GPS: OK \n");

  // Safety
  // -- Ensures that the system resets to a safe state (i.e. motors off)
  environmentMode = false;
  controlMode = false;
  demoMode = false;
  rudderPos = 6;
  speedSel = 0;
  Serial.print("SAFETY: OK \n");

  // Navigation
  lastCoordLat = 0.0;
  lastCoordLon = 0.0;
  Serial.print("NAVIGATION: OK \n");

  // IMU
  // -- Includes built in self-test.
  //    If the boot process ends after the print statement above, it is likely that the test has failed.
  byte c = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 "); Serial.print("IMU is at address: "); Serial.print(c, HEX);
  
  if (c == 0x71) {
    Serial.println("MPU9250 is online...");

    IMU.MPU9250SelfTest(IMU.SelfTest); // Self-test
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias); // Calibration
    IMU.initMPU9250(); // Initialise
    byte d = IMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963); // Test magnetometer
    IMU.initAK8963(IMU.magCalibration);
  }
  else {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);
  }
  Serial.print("IMU: OK \n");

  lastCoordLat = 56.160074;
  lastCoordLon = -3.072013;

  // Ready
  Serial.print("* * * READY * * * \n");
}

/**
 * This function will run continuously when no other function is running.
 */
void loop() {
  delay(1000);

  radarScan();

  // GPS
  gpsLoop();
  
  getDistance();

  Serial.println(getHeading());

  statusUpdate(); // Automatically updates the status of the nav-con.

  if (awaitingResponse == true && controlMode == true) {
    checkForResponse();
  }
  else { // This if statement determines what mode controller is activated.
    if(demoMode == true){
      demoModeController();
    }
    else if (environmentMode == false && controlMode == true) {
      outdoorAutoController();
    }
    else if (environmentMode == true && controlMode == true) {
      //indoorAutoController();
      demoModeController2();
    }
    else if(environmentMode == false && controlMode == false){
      manualModeController();
    }
    else{
      manualModeController();
    }
  }
}

/* Functions to operate the demonstration mode */

/**
 * Runs the demonstration mode for the hovercraft.
 * 
 * In this mode the hovercraft behaves according to a simple behaviour-based robotics (BBR) mechanism.
 * It will control the actuators in direct response to external stimulus.
 */
void demoModeController(){
  Serial.println("DEMO MODE");

  bool res = obstacleDetection();
  
  if (obstacleDirection[0] == true && obstacleDirection[1] == false && obstacleDirection[2] == false) {
    softStop();
    radarScan();
    turnByDegrees(true, 30);
    sudoMoveForward();
  }
  else if (obstacleDirection[0] == false && obstacleDirection[1] == false && obstacleDirection[2] == true) {
    softStop();
    radarScan();
    turnByDegrees(false, 30);
    sudoMoveForward();
  }
  else if (obstacleDirection[0] == false && obstacleDirection[1] == true && obstacleDirection[2] == false) {
    softStop();
    radarScan();
    turnByDegrees(true, 90);
    sudoMoveForward();
  }
  else if (obstacleDirection[0] == true && obstacleDirection[1] == true && obstacleDirection[2] == false) {
    softStop();
    radarScan();
    turnByDegrees(true, 45);
    sudoMoveForward();
  }
  else if (obstacleDirection[0] == false && obstacleDirection[1] == true && obstacleDirection[2] == true) {
    softStop();
    radarScan();
    turnByDegrees(false, 45);
    sudoMoveForward();
  }
  else if(obstacleDirection[0] == true && obstacleDirection[1] == true && obstacleDirection[2] == true){
    radarScan();
    softStop(); // Should realistically be emergency stop, but then it would not function indoors at all
  }
  else {
    sudoMoveForward();
  }
}

/**
 * Runs and alternative demonstration mode for the hovercraft.
 * 
 * Similar to the standard demo mode controller, but employing the obstacle avoidance strategy.
 * Slightly more intelligent.
 */
void demoModeController2(){
  Serial.println("DEMO MODE");

  bool res = obstacleDetection();
  
  if (obstacleDirection[0] == true || obstacleDirection[1] == true || obstacleDirection[2] == true) {
    softStop();
    navigateObstacle();
  }
  else {
    sudoMoveForward();
  }
}

/* Server communication handlers */

/**
 * Handles data received from the master.
 */
void receiveEvent(int byteCount) {

  int number = 0;
  int state = 0;

  //Serial.print(byteCount);

  // Some commands will not be handled directly here, but rather will be passed on to the
  // actuation controller if there is nothing to be done at this level.
  if (byteCount == 1) {
    while (Wire.available()) {
      number = Wire.read();

      if (number != 0) {
        commandQueue.enqueue(number);
      }
      lastCommand = number;

      if (number == 255) {
        Serial.print("ACK: RESET \n");
      }
      if (number == 1) {
        Serial.print("ACK: POWER ON \n");
      }
      if (number == 2) {
        Serial.print("ACK: POWER OFF \n");
      }
      if (number == 10) {
        Serial.print("ACK: AUTOMATIC MODE \n");
        controlMode = true;
      }
      if (number == 11) {
        Serial.print("ACK: MANUAL MODE \n");
        controlMode = false;
      }
      if (number == 12) {
        Serial.print("ACK: INDOOR MODE \n");
        environmentMode = true;
      }
      if (number == 13) {
        Serial.print("ACK: OUTDOOR MODE \n");
        environmentMode = false;
      }
      if (number == 100) {
        Serial.print("ACK: FORWARD 1U \n");
        if(demoMode == false){
          demoMode == true;
        }
        else{
          demoMode == false;
        }
      }
      if (number >= 101 && number <= 111) {
        Serial.print("ACK: RUDDER POS:");
        rudderPos = 101 - number;
        Serial.print(rudderPos);
        Serial.print("\n");
      }
      if (number == 112) {
        Serial.print("ACK: INVALID RUDDER POS:");
        Serial.print("\n");
      }
      if (number >= 120 && number <= 129) {
        Serial.print("ACK: MOTOR SPEED: ");
        speedSel = 120 - number;
        Serial.print(speedSel);
        Serial.print("\n");
      }
      if (number == 130) {
        Serial.print("ACK: INVALID SPEED SEL:");
        Serial.print("\n");
      }
      if (number == 140) {
        Serial.print("ACK: COORDINATE INCOMING \n");
        lastCommand = number;
      }
      if (number == 141) {
        Serial.print("ACK: INDOOR START COORDINATE INCOMING \n");
      }
      if (number == 142) {
        Serial.print("ACK: INDOOR DEST COORDINATE INCOMING \n");
      }
    }
  }

  // This handles situations where the user has submitted an indoor coordinate.
  // It will read the coordinate and parse it so that it can be stored in the local data strucutre.
  if (byteCount > 1 && (lastCommand == 141)) {
    lastIndoorStartCoord = "";
    while (1 < Wire.available()) {
      int c = Wire.read();
      lastIndoorStartCoord += c;
    }
    int x = Wire.read();
    lastIndoorStartCoord += x;

    String startCoordY = lastIndoorStartCoord.substring(1, 4);
    String startCoordX = lastIndoorStartCoord.substring(5);

    lastIndoorStartCoordY = startCoordY.toInt();
    lastIndoorStartCoordX = startCoordX.toInt();

    //Serial.println(lastIndoorStartCoord);
    Serial.print(lastIndoorStartCoordY);
    Serial.print(", ");
    Serial.print(lastIndoorStartCoordX);
    Serial.println();
  }
  
  if (byteCount > 1 && (lastCommand == 142)) {
    lastIndoorDestCoord = "";
    while (1 < Wire.available()) {
      int c = Wire.read();
      lastIndoorDestCoord += c;
    }
    int x = Wire.read();
    lastIndoorDestCoord += x;

    String destCoordY = lastIndoorDestCoord.substring(1, 4);
    String destCoordX = lastIndoorDestCoord.substring(5);

    lastIndoorDestCoordY = destCoordY.toInt();
    lastIndoorDestCoordX = destCoordX.toInt();

    //Serial.println(lastIndoorDestCoord);
    Serial.print(lastIndoorDestCoordY);
    Serial.print(", ");
    Serial.print(lastIndoorDestCoordX);
    Serial.println();
  }

  // This handles situations where the user has submitted an outdoor coordinate.
  // It will read the coordinate and parse it so that it can be stored in the local data strucutre.
  //
  // This is likely not the cleanest solution, but over the current interface it is able to obtain a
  // coordinate of relatively good accuracy.
  if (byteCount > 20) {
    while (1 < Wire.available()) { // loop through all but the last
      int c = Wire.read(); // receive byte as a character
      lastCoord += c;
      //Serial.print(c);         // print the character
    }
    int x = Wire.read();    // receive byte as an integer
    lastCoord += x;

    String coordLat = lastCoord.substring(1, 11);
    String coordLon = lastCoord.substring(12);

    Serial.println(coordLat);
    Serial.println(coordLon);

    String coordLat1 = coordLat.substring(0, 2);
    String coordLat2 = coordLat.substring(3);

    String coordLon1 = coordLon.substring(0, 2);
    String coordLon2 = coordLon.substring(3);

    Serial.print(coordLat1);
    Serial.print(".");
    Serial.println(coordLat2);

    String coordLatTmp = coordLat1 + "." + coordLat2;
    String coordLonTmp = coordLon1 + "." + coordLon2;

    float coordLatTmpFloat = coordLatTmp.toFloat();
    float coordLonTmpFloat = coordLonTmp.toFloat();

    lastCoordLat = coordLatTmpFloat;
    lastCoordLon = coordLonTmpFloat;

    Serial.print(coordLatTmpFloat, 6);
    Serial.print(",");
    Serial.println(coordLonTmpFloat, 6);

    lastCommand = 3;
  }
}

/**
 * This function is responsible for issuing information back to the server.
 * The information it provides is used to make sure that the status of the GUI matches
 * what is happening at the controller level.
 *
 * If the serial link is down this will cause problems with the GUI.
 */
void statusUpdate() {
  if (!controlMode)
  {
    Serial.println(1000);  // MANUAL
  }
  else
  {
    Serial.println(1001);  // AUTO
  }

  if (!environmentMode)
  {
    Serial.println(2000);  // OUTDOOR
  }
  else
  {
    Serial.println(2001);  // INDOOR
  }

  int speedStat = 3000 + speedSel;
  Serial.println(speedStat);

  int rudderStat = 4000 + rudderPos;
  Serial.println(rudderStat);
}

/**
 * Responds to request interrupts from the master.
 *
 * As of 23/02/17 this function is deprecated, as callbacks are
 * carried out using the Serial connection to the Raspberry Pi.
 */
void requestEvent() {
  //  switch (lastCommand) {
  //    case 1:
  //      Wire.write("Starting up.... ");
  //      break;
  //    case 2:
  //      Wire.write("Shutting down...");
  //      break;
  //    default:
  //      Wire.write("I2C: ACKNOWLEDGE");
  //    break;
  //  }
}

/* Actuation Controller communications */

/**
 * Sends the most recent command to the actuation controller, if there is one.
 */
void sendActuationCommand() {
  Wire.beginTransmission(3);
  if (commandQueue.count() > 0) {
    Serial.println(commandQueue.peek());
    Wire.write(commandQueue.dequeue());
    //Serial.print(commandQueue.dequeue());
    //Serial.print("\n");
  }
  else {
    Wire.write(0);
  }
  Wire.endTransmission();
}

/**
 * Polls the actuation controller to see if it has completed its
 * last command. Another command will not be issued until it has.
 */
void checkForResponse() {
  Wire.requestFrom(3, 1);

  while (Wire.available()) {
    int check = Wire.read();
    Serial.print("Actation controller status: ");
    Serial.println(check);
    if (check == 250) { // 250 = done
      awaitingResponse = false;
    }
  }
}

/**
 * Carries out the emrgency stop proceedure.
 */
void emergencyStop() {
  // Instruct actuation controller to stop
  commandQueue.enqueue(2);
  commandQueue.enqueue(120);

  // Reset GUI
  speedSel = 0;
  rudderPos = 6;
}

/**
 * Shuts down the rear propellor to slow down.
 */
void softStop() {
  Serial.println("softStop()");
  commandQueue.enqueue(120);
}

/* Indoor/Outdoor - Manual functions */

/**
 * This is the mode controller for the manual mode.
 */
void manualModeController() {
  Serial.println("MANUAL MODE");

  sendActuationCommand();

  if (obstacleDetection()) {
    if (obstacleCritical) {
      //emergencyStop();
      Serial.print("[NAV-CON] EMERGENCY STOP! Critical obstacle detected. You must press OFF and then ON to continue. \n");
    }
    if (obstacleDirection[0] == true) {
      //softStop();
      Serial.print("[NAV-CON] Obstacled detected to LEFT of vehicle. Adjust rudder accordingly before continuing. \n");
    }
    if (obstacleDirection[1] == true) {
      //softStop();
      Serial.print("[NAV-CON] Obstacled detected to FRONT of vehicle. Adjust rudder accordingly before continuing. \n");
    }
    if (obstacleDirection[2] == true) {
      //softStop();
      Serial.print("[NAV-CON] Obstacled detected to RIGHT of vehicle. Adjust rudder accordingly before continuing. \n");
    }
  }
}

/**
 * Sends a turn command to the actuation controller of a specific
 * direction and number of degrees.
 *
 * numDegrees should not exceed 180.
 */
void turnByDegrees(bool direction, int numDegrees) {
  if(direction == false){
    Serial.print("Turning LEFT by ");
    Serial.print(numDegrees);
    Serial.print(" degrees \n");
  }
  else{
    Serial.print("Turning RIGHT by ");
    Serial.print(numDegrees);
    Serial.print(" degrees \n");
  }
  
  // The actuation controller expects a packet containing the 
  // direction followed by the number of degrees.
  byte buffer[2];
  
  Wire.beginTransmission(3);
  numDegrees = numDegrees + numDegrees;
  if (direction == false) {
    buffer[0] = 201;
    buffer[1] = numDegrees;
    Wire.write(buffer, 2);
  }
  else {
    buffer[0] = 202;
    buffer[1] = numDegrees;
    Wire.write(buffer, 2);
  }
  Wire.endTransmission();
  awaitingResponse = true; // This locks sendActuationCommand to avoid overloading act-con
}

/**
 * Moves the hovercraft forward until it detects an obstacle. Calls
 * the obstacle evasion functions if obstacle detected, which runs before
 * returning to the main controller.
 */
void moveForward() {
  Serial.println("moveForward()");
  if (obstacleDetection()) {
    Serial.println("moveForward(): obstacle detected");
    //radarScan();
    bool res = navigateObstacle();
    if(!res){
      Serial.print("[NAV-CON] Unable to circumnavigate obstacle, please manually move the vehicle! \n");
    }
  }
  else {
    Wire.beginTransmission(3);
    Wire.write(100); // 100 = forward at steady pace
    Wire.endTransmission();
  }
}

/**
 * Moves the hovercraft forward for a short burst regardless of obstacle detection status.
 */
void sudoMoveForward() {
  Serial.println("sudoMoveForward()");
  Wire.beginTransmission(3);
  Wire.write(100);
  Wire.endTransmission();
}

/* Outdoor - Auto functions */

/**
 * This is the mode controller for the Outdoor - Auto mode.
 *
 * It instantiates structs which hold the current and destination
 * coordinates. The current position is retrieved from the GPS.
 *
 * If the user has provided a destination coordinate via the GUI,
 * navigation will begin, otherwise it will be stuck in the loop waiting.
 */
void outdoorAutoController() {

  if (controlMode == false) {
    return;
  }

  double destinationHeading, currentHeading;
  coord currentPosition, destinationPosition;
  
  Serial.println(getLatitude());
  Serial.println(getLongitude());

  currentPosition = getCurrentPosition();

  if (lastCoordLat == 0.0 && lastCoordLon == 0.0) {
    Serial.print("[NAV-CON] Please provide a destination coordinate... \n");
  }
  else if(GPS.fix == 0){
    Serial.print("Waiting for a GPS fix... \n");
    delay(1000);
  }
  else {
    currentPosition = getCurrentPosition();

    destinationPosition = getDestinationPosition();

    currentHeading = getHeading(); // From IMU compass
    //currentHeading = 150.0;
    destinationHeading = getDestinationHeading(&destinationPosition, &currentPosition);

    calcMoveToHeading(destinationHeading, currentHeading);

    moveForward();
  }
}

/**
 * Retreives the current position from the GPS module.
 */
coord getCurrentPosition() {
  coord currentPosition;

  //currentPosition.lat = 56.191834;
  //currentPosition.lon = -3.145690;

  currentPosition.lat = getLatitude();
  currentPosition.lon = getLongitude();

  return currentPosition;
}

/**
 * Retreives the destination coordinates.
 */
coord getDestinationPosition() {
  coord destinationPosition;

  //destinationPosition.lat = 56.160074;
  //destinationPosition.lon = -3.072013;

  destinationPosition.lat = lastCoordLat;
  destinationPosition.lon = lastCoordLon;

  return destinationPosition;
}

/**
 * Retreives the vehicles current heading from the IMU compass.
 *
 * If a reliable sensor is available to provide the current heading the
 * code for it would go here, until then the value is set manually.
 */
double getCurrentHeading() {
  double currentHeading;

  currentHeading = 180;

  return currentHeading;
}

/**
 * Implements the bug algorithm to circumnavigate the obstacle.
 */
bool navigateObstacle() {
  Serial.println("navigateObstacle()");
  bool res;
  long duration;
  bool turnDirection;
  bool correctionRequired = false;

  int correctionDegrees = 0;

  obstacleDetection();
  if (obstacleDirection[0] == true && obstacleDirection[1] == false && obstacleDirection[2] == false) {
    turnByDegrees(true, 30);
    correctionDegrees = 30;
    sudoMoveForward();

    radarServo.write(180);
    turnDirection = false;
    correctionRequired = true;
  }
  else if (obstacleDirection[0] == false && obstacleDirection[1] == false && obstacleDirection[2] == true) {
    turnByDegrees(false, 30);
    correctionDegrees = 30;
    sudoMoveForward();

    radarServo.write(10);
    turnDirection = true;
    correctionRequired = true;
  }
  else if (obstacleDirection[0] == false && obstacleDirection[1] == true && obstacleDirection[2] == false) {
    turnByDegrees(true, 90);
    correctionDegrees = 90;

    radarServo.write(180);
    turnDirection = false;
    correctionRequired = true;
  }
  else if (obstacleDirection[0] == true && obstacleDirection[1] == true && obstacleDirection[2] == false) {
    turnByDegrees(true, 45);
    correctionDegrees = 45;
    sudoMoveForward();

    radarServo.write(180);
    turnDirection = false;
    correctionRequired = true;
  }
  else if (obstacleDirection[0] == false && obstacleDirection[1] == true && obstacleDirection[2] == true) {
    turnByDegrees(false, 45);
    correctionDegrees = 45;
    sudoMoveForward();

    radarServo.write(10);
    turnDirection = true;
    correctionRequired = true;
  }
  else {
    Serial.println("CANNOT NAVIGATE THIS OBSTACLE!");
    res = false;
    return res;
  }

  // Here we use the sonar to point to the left or right of the hovercraft, so as to
  // 'track' the obstacle. Distance values from the sonar are averaged to improve reliability.
  long d1, d2, d3;
  long dAvg;

  d1 = 0; d2 = 0; d3 = 0;
  dAvg = 0;
  
  while(dAvg < 60){
    digitalWrite(radarTrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(radarTrigPin, HIGH);
    delayMicroseconds(10); // 10us
    digitalWrite(radarTrigPin, LOW);
    duration = pulseIn(radarEchoPin, HIGH);

    d3 = d2;
    d2 = d1;
    d1 = microsecondsToCentimeters(duration);

    dAvg = (d1 + d2 + d3) / 3;

    sudoMoveForward();
    
    Serial.print("Radar distance: ");
    Serial.println(dAvg);
  }
  if(correctionRequired == true){
    if(turnDirection == false){ // TURNED LEFT
      turnByDegrees(false, correctionDegrees);
      sudoMoveForward();
      res = true;
    }
    if(turnDirection == true){ // TURNED RIGHT
      turnByDegrees(true, correctionDegrees);
      sudoMoveForward();
      res = true;
    }
  }
  
  return res;
}

/**
 * Retreives the vehicles destination heading, based on the current coordinate position of the vehicle.
 *
 * Based on the current and destination coordinates, it is possible to calculate the degree that the hovercraft
 * must align with in order to travel directly towards the destination.
 *
 * Takes into account the curvature of the earth.
 */
double getDestinationHeading(coord* currentPosition, coord* destinationPosition) {
  int moveEastWest = 0;
  int moveNorthSouth = 0;
  double destinationHeading = 0;
  double currentHeading = 0;

  double a = destinationPosition->lat * pi / 180;
  double b = destinationPosition->lon * pi / 180;
  double c = currentPosition->lat * pi / 180;
  double d = currentPosition->lon * pi / 180;

  if (cos(c) * sin(d - b) == 0) {
    if (c > a) {
      destinationHeading = 0;
    }
    else {
      destinationHeading = 180;
    }
  }
  else {
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
 * Determine the number of degrees to turn and in which direction.
 *
 * Calculates the most efficient direction to turn once we know the heading that
 * must be followed.
 */
void calcMoveToHeading(double destinationHeading, double currentHeading) {
  int degDifference = destinationHeading - currentHeading;
  int degTurn;
  boolean turnLeftNotRight = false;

  if (-180 < degDifference && degDifference <= 180) {
    degTurn = degDifference;
  }
  if (degDifference > 180) {
    degTurn = degDifference - 360;
  }
  if (degDifference <= -180) {
    degTurn = degDifference + 360;
  }

  if (degTurn > 0) {
    turnLeftNotRight = true;
  }
  if (degTurn < 0) {
    turnLeftNotRight = false;
  }

  if(degTurn > (previousDeg - 20) && degTurn < (previousDeg + 20)){
    Serial.println("No need to turn.");
  }
  else{
    if (turnLeftNotRight == false) {
      Serial.print("Turning left: ");
      Serial.print(degTurn);
      Serial.print("\n");
      turnByDegrees(false, degTurn);
      previousDeg = degTurn;
    }
    if (turnLeftNotRight == true) {
      Serial.print("Turning right: ");
      Serial.print(degTurn);
      Serial.print("\n");
      turnByDegrees(true, degTurn);
      previousDeg = degTurn;
    } 
  }

  return;
}

/* Indoor - Auto functions */

/**
 * This is the mode controller for the Indoor - Auto mode.
 *
 * TODO
 */
void indoorAutoController() {

}

/* Obstacle Detection */

/**
 * Checks if it is safe to move forward, returns false if yes.
 */
bool obstacleDetection() {
  Serial.println("obstacleDetection()");
  bool res = false;

  long distance1, distance2, distance3;
  long duration1, duration2, duration3;

  int i;
  for (i = 0; i < 3; i++) {
    obstacleDirection[i] = false;
  }

  digitalWrite(distLeftTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(distLeftTrig, HIGH);
  delayMicroseconds(10); // 10us
  digitalWrite(distLeftTrig, LOW);
  duration1 = pulseIn(distLeftEcho, HIGH);
  distance1 = microsecondsToCentimeters(duration1);

  Serial.print("DISTANCES - Left: ");
  Serial.print(distance1);

  delay(10);

  digitalWrite(distCenterTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(distCenterTrig, HIGH);
  delayMicroseconds(10); // 10us
  digitalWrite(distCenterTrig, LOW);
  duration2 = pulseIn(distCenterEcho, HIGH);
  distance2 = microsecondsToCentimeters(duration2);

  Serial.print(", Centre: ");
  Serial.print(distance2);

  delay(10);

  digitalWrite(distRightTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(distRightTrig, HIGH);
  delayMicroseconds(10); // 10us
  digitalWrite(distRightTrig, LOW);
  duration3 = pulseIn(distRightEcho, HIGH);
  distance3 = microsecondsToCentimeters(duration3);

  Serial.print(", Right: ");
  Serial.println(distance3);

  if (distance1 < 100) {
    obstacleDirection[0] = true;
    res = true;
  }
  if (distance2 < 100) {
    obstacleDirection[1] = true;
    res = true;
  }
  if (distance3 < 100) {
    obstacleDirection[2] = true;
    res = true;
  }

  if (obstacleDirection[0] == true && obstacleDirection[1] == true && obstacleDirection[2] == true) {
    obstacleCritical = true;
  }

  return res;
}

/* Radar */

/**
 * Completes one complete 180* radar scan and saves the distances to an array.
 *
 * TODO: Alex, please comment!
 */
void radarScan() {
  Serial.println("radarScan()");
  
  long duration;
  long distance;

  for (radar_angle = 0; radar_angle <= 180; radar_angle = radar_angle + 10) {
    radarServo.write(radar_angle);

    pos = radar_angle / 10;
    //maparray [prevX[pos]][prevY[pos]] = false;

    digitalWrite(radarTrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(radarTrigPin, HIGH);
    delayMicroseconds(10); // 10us
    digitalWrite(radarTrigPin, LOW);
    duration = pulseIn(radarEchoPin, HIGH);
    distance = microsecondsToCentimeters(duration);

    // Store
    Xaxis = (distance * sin(radar_angle) + 25) / 10;           // get x coordinate according to trigonometry (+50 for placement along the axis)
    Yaxis = (distance * cos(radar_angle)) / 10;                // get y coordinate ''
    //maparray [Xaxis][Yaxis] = true;                            // place obstacle in map array
    prevX[pos] = Xaxis;                                        // store coordinates in array according to servo position
    prevY[pos] = Yaxis;

    if (distance < 200) {
      radarDistances[(radar_angle / 10)] = distance;
      delay(200);
    }
    else {
      radarDistances[radar_angle / 10] = 200;
      //Serial.print("[RADAR] Angle: %d, Distance: OUT OF RANGE \n", radar_angle);
    }
  }

  int temp[18];

  int i = 0;
  int j = 18;

  while(j){
    temp[i] = radarDistances[j];
    i++;
    j--;
  }

  for(int i = 0; i < 18; i++){
    radarDistances[i] = temp[i];
  }

  Serial.print("[RADAR] ");
  for(int i = 0; i < 18; i++){
    Serial.print(radarDistances[i]);
    Serial.print(", ");
  }
  Serial.print("\n");

  radarCalcDirection();
}

/**
 * Prints the current set of distances from 0-180*.
 */
void radarPrintDistances() {
  String msg = "[RADAR] ";
  for (int i = 0; i <= 18; i++) {
    msg += radarDistances[i];
    msg += " ";
  }
  msg += "\n";
  Serial.print(msg);
  return;
}

/**
 * Calculates best direction of travel based on obstacles detected by scanning radar.
 *
 * Based on the current values held in the sonar array, this function will work out if there is
 * a space through which it will fit and will indicate what direction to travel on that basis.
 */
void radarCalcDirection() {
  int possibleDirection  = 0;
  int radarClear = 0;
  int bestRadarClear = 0;
  bool directionFound = false;
  bool clearDirections[18];
  bestPossibleDirection = 0;

  for (int i = 0; i <= 18; i++) {
    if (radarDistances[i] >= 60) {
      clearDirections[i] = true;
    }
    else {
      clearDirections[i] = false;
    }
  }

  // Searching for a continuous gap in the sonar reading.
  for (int i = 0; i <= 16; i++) {
    if ((clearDirections[i] == true) && (clearDirections[i + 1] == true) && (clearDirections[i + 2] == true)) {
      directionFound = true;
      possibleDirection = ((i + 1) * 10);
      radarClear = radarDistances[i + 1];
      if (radarClear > bestRadarClear) {
        bestPossibleDirection = possibleDirection;
        bestRadarClear = radarClear;
      }
    }
  }

  if (directionFound == false) {
    bestPossibleDirection = -1;
  }

  Serial.print("Best direction to navigate: ");
  Serial.print(bestPossibleDirection);
  Serial.print("\n");
}

/**
 * Helper function to convert time into distance travelled.
 */
long microsecondsToCentimeters(long us) {
  return us / 58;
}

/**
 * IMPORTANT
 *
 * The functions below this line are mostly implemented as instructed in the datasheet for the sensors.
 * There is documentation online for most of the sensors that provide further analysis of what the code does.
 */

/* GPS */

/**
 * Interrupt routine, runs when GPS sends an update.
 */
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;
#endif
}

/**
 * Sets up the interrupt function to allow asynchronus GPS updating.
 */
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

/**
 * Polls the interrupt status to see if new data is available, and prints it.
 */
void gpsLoop() {

//  if(usingInterrupt){
//    delay(1000);
//    useInterrupt(false);
//  }

  Serial.println();
  Serial.println("* * * * * BEGIN GPS LOOP * * * * *");
  
  if (!usingInterrupt) {
    char c = GPS.read();
    if (GPSECHO)
      if (c) Serial.print(c);
  }

  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }

  if (timer > millis())  timer = millis();
  
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Location (in degrees, works with Google Maps): ");
      Serial.print(GPS.latitudeDegrees, 4);
      Serial.print(", "); 
      Serial.println(GPS.longitudeDegrees, 4);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }

  Serial.println("* * * * * END GPS LOOP * * * * *");
}

/**
 * Returns GPS latitude.
 */
float getLatitude () {
  if (GPS.fix) {
    return GPS.latitudeDegrees;
  }
  return 0;
}

/**
 * Returns GPS longitude.
 */
float getLongitude () {
  if (GPS.fix) {
    return GPS.longitudeDegrees;
  }
  return 0;
}

/**
 * Returns GPS angle.
 */
float getAngle () {
  if (GPS.fix) {
    return GPS.angle;
  }
  return 0;
}

/* IMU */

/**
 * Returns the current heading from the IMU's compass.
 */
double getHeading() {
  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {
    IMU.readMagData(IMU.magCount);  // Read the x/y/z adc values
    IMU.getMres();

    IMU.magbias[0] = xmbias();
    IMU.magbias[1] = ymbias();

    IMU.mx = (float)IMU.magCount[0] * IMU.mRes * IMU.magCalibration[0] - IMU.magbias[0];
    IMU.my = (float)IMU.magCount[1] * IMU.mRes * IMU.magCalibration[1] - IMU.magbias[1];

  }

  IMU.updateTime();

  MahonyQuaternionUpdate(IMU.ax, IMU.ay, IMU.az, IMU.gx * DEG_TO_RAD,
                         IMU.gy * DEG_TO_RAD, IMU.gz * DEG_TO_RAD, IMU.my,
                         IMU.mx, IMU.mz, IMU.deltat);

  // Serial print and/or display at 0.5 s rate independent of data rates
  IMU.delt_t = millis() - IMU.count;

  //if (IMU.delt_t > 500) {
    Serial.print("mx = "); Serial.print( (int)IMU.mx );
    Serial.print(" my = "); Serial.print( (int)IMU.my );
    Serial.println(" mG");

    IMU.count = millis();
    IMU.sumCount = 0;
    IMU.sum = 0;

    if (IMU.my == 0) {
      if (IMU.mx > 0) {
        lastKnownHeading = 0;
      }
      else {
        lastKnownHeading = 180;
      }
    }
    else {
      if (IMU.my > 0) {
        lastKnownHeading = (90 - (atan(IMU.mx / IMU.my) * 180 / pi));
      }
      else {
        lastKnownHeading = (270 - (atan(IMU.mx / IMU.my) * 180 / pi));
      }
      lastKnownHeading = 360 - lastKnownHeading;
    }
    if (lastKnownHeading >= 180) {
      lastKnownHeading = lastKnownHeading - 180;
    }
    else {
      lastKnownHeading = lastKnownHeading + 180;
    }
  //}

  return lastKnownHeading;
}

/**
 * Returns bias for magnetic X axis.
 */
int xmbias () {
  MagX = (float)IMU.magCount[0] * IMU.mRes * IMU.magCalibration[0];

  if (mxcalmax < MagX) {
    mxcalmax = MagX;
  }
  if (mxcalmin > MagX) {
    mxcalmin = MagX;
  }

Serial.print ("Bias X: "); Serial.println ((mxcalmax + mxcalmin) / 2); 
  return (mxcalmax + mxcalmin) / 2;
}

/**
 * Returns bias for magnetic Y axis.
 */
int ymbias () {

  MagY = (float)IMU.magCount[1] * IMU.mRes * IMU.magCalibration[1];

  if (mycalmax < MagY) {
    mycalmax = MagY;
  }
  if (mycalmin > MagY) {
    mycalmin = MagY;
  }

  Serial.print ("Bias Y: "); Serial.println ((mycalmax + mycalmin) / 2); 
  return (mycalmax + mycalmin) / 2;
}

double getDistance () {

  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    IMU.readAccelData(IMU.accelCount);  // Read the x/y/z adc values
    IMU.getAres();
    IMU.ax = (float)IMU.accelCount[0] * IMU.aRes;
  }

  double finalaccelvelocity;
  double distance;
  double curacctime = millis();
  double accelTime = (curacctime - prevacctime) / 1000;
  Serial.print("Time: "); Serial.print(millis() / 1000); Serial.println ("s");

  IMU.updateTime();

  MahonyQuaternionUpdate(IMU.ax, IMU.ay, IMU.az, IMU.gx * DEG_TO_RAD, IMU.gy * DEG_TO_RAD, IMU.gz * DEG_TO_RAD, IMU.my, IMU.mx, IMU.mz, IMU.deltat);

  // Serial print and/or display at 0.5 s rate independent of data rates
  IMU.delt_t = millis() - IMU.count;

  if (IMU.delt_t > 500) {
    IMU.count = millis();
    IMU.sumCount = 0;
    IMU.sum = 0;


    IMU.ax = -1 * IMU.ax;

    if (IMU.ax > - 0.075 && IMU.ax < 0.075) {
      IMU.ax = 0.00;
    }

    Serial.print("X-acceleration: "); Serial.print(IMU.ax);
    Serial.println(" g ");

    Serial.print("Distance between "); Serial.print(prevacctime); Serial.print(" - "); Serial.print(curacctime); Serial.print("ms: ");
    finalaccelvelocity = accelvelocity + (IMU.ax * accelTime);
    distance = (accelvelocity * accelTime) + (0.5 * IMU.ax * accelTime * accelTime);
    Serial.print(distance); Serial.println("m");

    Serial.print("Total distance: ");
    totaldistance = distance + totaldistance;
    Serial.print(totaldistance); Serial.println("m");

    prevacctime = curacctime;
    accelvelocity = finalaccelvelocity;
  }
  return totaldistance;
}
