/* Created by ScottC on 10 Jan 2013 
 http://arduinobasics.blogspot.com/2013/01/arduino-basics-sonar-project-tutorial.html
*/

import processing.serial.*;

int distance;
int angle=0;
int direction=1;

int[] alphaVal = new int[100]; // used to fade the lines
int[] distance2 = new int[100]; // used to store the line lengths
int lineSize = 4; // line length multiplier (makes it longer)

String comPortString;
Serial comPort;

/*---------------------SETUP---------------------------*/
void setup( ) {
 size(displayWidth,displayHeight); //allows fullscreen view
 smooth();
 background(0); // set the background to black
 
 /*Open the serial port for communication with the Arduino
 Make sure the COM port is correct - I am using COM port 8 */
 comPort = new Serial(this, Serial.list()[3], 9600);
 comPort.bufferUntil('\n'); // Trigger a SerialEvent on new line 
 
 /*Initialise the line alphaValues to 0 (ie not visible) */
 for(int i=0; i<91; i++){
 alphaVal[i] = 0;
 }
}
 
/*---------------------DRAW-----------------*/
void draw( ) {
 background(0); //clear the screen
 
 /*Draw each line and dot */
 for(int i=0; i<91; i++){
 
 /*Gradually fade each line */
 alphaVal[i]=alphaVal[i]-4;
 
 /*Once it gets to 0, keep it there */
 if(alphaVal[i]<0){
 alphaVal[i]=0;
 }
 
 /*The colour of the line will change depending on the distance */
 stroke(255,distance2[i],0,alphaVal[i]);
 
 /* Use a line thickness of 2 (strokeweight) to draw the line that fans
 out from the bottom center of the screen. */
 strokeWeight(2);
 line(width/2, height, (width/2)-cos(radians(i*2))*(distance2[i]*lineSize), height-sin(radians(i*2))*(distance2[i]*lineSize));
 
 /* Draw the white dot at the end of the line which does not fade */
 stroke(255);
 strokeWeight(1);
 ellipse((width/2)-cos(radians(i*2))*(distance2[i]*lineSize), height-sin(radians(i*2))*(distance2[i]*lineSize),5,5);
 }
}

/* A mouse press starts the scan. There is no stop button */
void mousePressed(){
 sendAngle();
}

/*When the computer receives a value from the Arduino, it will update the line positions */
void serialEvent(Serial cPort){
 comPortString = cPort.readStringUntil('\n');
 if(comPortString != null) {
 comPortString=trim(comPortString);
 
 /* Use the distance received by the Arduino to modify the lines */
 distance = int(map(Integer.parseInt(comPortString),1,200,1,height));
 drawSonar(angle,distance);

 /* Send the next angle to be measured by the Arduino */ 
 sendAngle();
 }
}

/*---------------------------sendAngle() FUNCTION----------------*/
void sendAngle(){
 //Send the angle to the Arduino. The fullstop at the end is necessary.
 comPort.write(angle+".");
 
 /*Increment the angle for the next time round. Making sure that the angle sent
 does not exceed the servo limits. The "direction" variable allows the servo
 to have a sweeping action.*/
 angle=angle+(2*direction);
 if(angle>178||angle<1){
 direction=direction*-1;
 }
}

/*-----------------sketchFullScreen(): Allows for FullScreen view------*/
//boolean sketchFullScreen() {
 //return true;
//}

/*----------------- drawSonar(): update the line/dot positions---------*/
void drawSonar(int sonAngle, int newDist){
 alphaVal[sonAngle/2] = 180;
 distance2[sonAngle/2] = newDist;
}

 