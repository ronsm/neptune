/*
 * outdoorAuto
 * -----------
 * Autonomous Otdoor mode for Neptune's Navigation Controller
 * 
 * Author: Ronnie Smith
 * Version: 0.1
 * Date: 19/12/2016
 * 
 */

// Global constant and variable declarations
int initialLoop = 0;

int currentDistances[4];

float destinationCoordinateLat, destinationCoordinateLon;
float currentCoordinateLat, currentCoordinateLon;

double destinationHeading, currentHeading;

const float pi = 3.14159;

void setup() {
  Serial.begin(19200);
}

void loop() {
  outdoorAutoController();
}

// Flow controllers

void outdoorAutoController(){

  if(initialLoop == 0){
    Serial.println("Enter the DESTINATION coordinate");
    readCoordinate(0);
    Serial.println("Enter the CURRENT coordinate");
    readCoordinate(1);
    
    printPositions();
    
    initialLoop = 1; // Stop this if statement from executing again
  }

  //getDistances();
  //printDistances();

  getDestinationHeading();
  getCurrentHeading();

  moveToHeading();
  
}

// Outdoor navigation control functions

void getDestinationHeading(){
  int moveEastWest = 0;
  int moveNorthSouth = 0;
  destinationHeading = 0;

  double a = destinationCoordinateLat * pi / 180;
  double b = destinationCoordinateLon * pi / 180;
  double c = currentCoordinateLat * pi / 180;
  double d = currentCoordinateLon * pi / 180;

  if (cos(c) * sin(d - b) == 0){
    if (c > a){
      destinationHeading = 0;   
    }
    else{
      destinationHeading = 180;
    }
  }
  else
  {
    double angle = atan2(cos(c) * sin(d - b), sin(c) * cos(a) - sin(a) * cos(c) * cos(d - b));
    double exp1 = (angle * 180 / pi + 360);
    double exp2 = 360;
    destinationHeading =  fmod(exp1, exp2);
  }

  Serial.print("Moving in direction: ");
  Serial.println(destinationHeading, DEC);

  return;
}

void getCurrentHeading(){
  // Get the current heading from the IMU

  return
}

void moveToHeading(){
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

// Functions to read data from serial

void getDistances(){
  Serial.println("Enter distance for Front sensor");
  readDistance(0);
  Serial.println("Enter distance for Right sensor");
  readDistance(1);
  Serial.println("Enter distance for Rear sensor");
  readDistance(2);
  Serial.println("Enter distance for Left sensor");
  readDistance(3);

  return;
}

void readCoordinate(int coordinateSelect){  
  // Lattitude
  Serial.println("Latitude:");
  
  while(Serial.available() == 0){
    delay(10);
  }
  
  if (Serial.available() > 0) {
    if(coordinateSelect == 0){
      destinationCoordinateLat = Serial.parseFloat();
    }
    if(coordinateSelect == 1){
      currentCoordinateLat = Serial.parseFloat();
    }
  }

  // Longitude
  Serial.println("Longitude:");
  
  while(Serial.available() == 0){
    delay(10);
  }
  
  if (Serial.available() > 0) {
    if(coordinateSelect == 0){
      destinationCoordinateLon = Serial.parseFloat();
    }
    if(coordinateSelect == 1){
      currentCoordinateLon = Serial.parseFloat();
    }
  }

  return;
}

void readDistance(int sensor){
  
  while(Serial.available() == 0){
    delay(10);
  }
  if (Serial.available() > 0) {
        currentDistances[sensor] = Serial.parseInt();
  }

  return;
}

// Function to print data to serial

void printPositions(){
  Serial.print("Current position: ");
  Serial.print(currentCoordinateLat, DEC);
  Serial.print(", ");
  Serial.print(currentCoordinateLon, DEC);
  Serial.print("\n");

  Serial.print("Destination position: ");
  Serial.print(destinationCoordinateLat, DEC);
  Serial.print(", ");
  Serial.print(destinationCoordinateLon, DEC);
  Serial.print("\n");

  return;
}

void printDistances(){
  Serial.print("Front: ");
  Serial.print(currentDistances[0], DEC);
  Serial.print(", Right: ");
  Serial.print(currentDistances[1], DEC);
  Serial.print(", Rear: ");
  Serial.print(currentDistances[2], DEC);
  Serial.print(", Left: ");
  Serial.print(currentDistances[3], DEC);
  Serial.print("\n");

  return;
}

