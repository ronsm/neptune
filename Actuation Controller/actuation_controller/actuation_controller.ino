/**
 * actuation_controller.ino
 *
 * This is the primary source file for the Neptune on-board actuation controller.
 * It is repsonsible for executing commands related to the movement of the vehicle.
 *
 * LICENSE: This source file is subject to a Creative Commons
 * Attribution-NonCommercial 4.0 International (CC BY-NC 4.0) License.
 * Full details of this license are available online at:
 * https://creativecommons.org/licenses/by-nc/4.0/.
 *
 * @package    Neptune
 * @author     Seb Smith
 * @copyright  2017, Seb Smith
 * @license    https://creativecommons.org/licenses/by-nc/4.0/ CC BY-NC 4.0
 * @version    1.1
 * @link       https://github.com/ronsm/neptune
 * 
 * ATTRIBUTIONS: This project uses and derives open source code and packages from
 * various authors, which are attributed here where possible.
 *    1) REF
 */
#include <Servo.h>
#include <Wire.h>

int incomingByte = 0;        //For incoming serial data

bool taskComplete = true;
bool forward = false;

int analoglevel = 0;
int batterylevel = 0;
int cellPin = A3;

int middlePosition = 105;    //Calibrate to servo middle position

int x;      //Used for storing bytes when 1 byte received

int t1, t2; //Used for sotrying bytes when 2 bytes received

Servo esc1;
Servo esc2;
Servo servo1;

void setup() {      
          Wire.begin(3);                // join i2c bus with address #3
          Wire.onReceive(receiveEvent); // register event
          Wire.onRequest(requestEvent);
          Serial.begin(9600);           // opens serial port, sets data rate to 9600 bps
          servo1.attach(3);
          esc1.attach(10);
          esc2.attach(6);

          //Initialisation of speed controllers and reset to default values
          esc1.write(20);
          esc2.write(20);
          servo1.write(middlePosition);              // Rudder at middle position
          delay(1000);
          
          Serial.println("Intialisation complete!");
}

void loop() {
  
  delay(100);

  analoglevel = analogRead(cellPin);
  batterylevel = map(analoglevel, 740, 860, 231, 241);      //Maps battery value between 0 and 255


    //Turn left N degrees
    if(t1 == 201){
      taskComplete = false;
      Serial.print("Calling turn function with degrees: "); 
      Serial.println(t2); 
      turnleft(t2);
    }
    //Turn right N degrees
    else if(t1 == 202){
      taskComplete = false;
      Serial.print("Calling turn function with degrees: "); 
      Serial.println(t2); 
      turnright(t2);
    }

    if (forward == true){
        forward1u();
        forward = false;
    }

    t1 = 0; t2 = 0;
  

  //Serial.print("Battery voltage is: ");
  //Serial.println(analoglevel);

  /* Battery voltage values:
   * 
   * Percentage  Voltage   Integer
   * 100% ------ 4.20V ---- 860
   * 75%  ------ 4.02V ---- 822
   * 50%  ------ 3.85V ---- 788
   * 42%  ------ 3.83V ---- 784
   * 30%  ------ 3.79V ---- 775
   * 11%  ------ 3.70V ---- 757
   * 0%   ------ 3.62V ---- 740
   */

}

void turnleft(int Ndegrees){
  Serial.print("Turning left: ");
  Serial.print(Ndegrees);
  Serial.println("degrees.");

  //Procedure for interrupt
  int delayamount = map (Ndegrees, 0, 180, 600, 2600); //Maps 0 to 180 to 0 to 2 seconds
  servo1.write(145);                    //Point rudder left the required ammount
  esc1.write(120);                       //Add some more throttle
  delay(delayamount);                   //Wait time to move required degrees
  servo1.write(middlePosition);         //Return rudder back to middle
  esc1.write(20);                       //Stop thrust prop
  taskComplete = true;                  //Finish control interrupt
  Serial.println ("Procedure complete");
}

void turnright(int Ndegrees){
  Serial.print("Turning right: ");
  Serial.print(Ndegrees);
  Serial.println("degrees.");

  //Procedure for interrupt
  int delayamount = map (Ndegrees, 0, 180, 600, 2600); //Maps 0 to 180 to 0 to 2 seconds
  servo1.write(65);                     //Point rudder left the required ammount
  esc1.write(120);                       //Add some more throttle
  delay(delayamount);                   //Wait time to move required degrees
  servo1.write(middlePosition);         //Return rudder back to middle
  esc1.write(20);                       //Stop thrust prop
  taskComplete = true;                  //Finish control interrupt

  Serial.println ("Procedure complete");
}

void poweron(){
   Serial.println("Power ON");
   esc2.write(90);
}

void forward1u(){
  Serial.println("Move forward 1 unit!");
  esc1.write(94);
  delay(1000);
  esc1.write(20);
}

void poweroff(){
   Serial.println("Power OFF");
   esc2.write(20);
   esc2.write(20);
   servo1.write(middlePosition); 
}

void manualmotor (int level){
 Serial.print("MOTOR manual change:");
 Serial.println(level);
 if (level == 120){esc1.write(20);}
 if (level == 121){esc1.write(40);}
 if (level == 122){esc1.write(60);}
 if (level == 123){esc1.write(80);}
 //if (level == 124){esc1.write(90);}
 //if (level == 125){esc1.write(100);}
 //if (level == 126){esc1.write(120);}
 //if (level == 127){esc1.write(140);}
 //if (level == 128){esc1.write(160);}
 //if (level == 129){esc1.write(180);}  
}

void manualservo (int level){
  Serial.print("SERVO manual change:");
  Serial.println(level);
  if (level == 101){servo1.write(155);}
  if (level == 102){servo1.write(145);}
  if (level == 103){servo1.write(135);}
  if (level == 104){servo1.write(125);}
  if (level == 105){servo1.write(115);}
  if (level == 106){servo1.write(105);}
  if (level == 107){servo1.write(95);}
  if (level == 108){servo1.write(85);}
  if (level == 109){servo1.write(75);}
  if (level == 110){servo1.write(65);}
  if (level == 110){servo1.write(55);}   
}

void receiveEvent(int howMany){

  if(howMany == 1){
   
    Serial.print("Single byte data received: ");
    x = Wire.read();           // receive byte as an integer
    Serial.println(x);         // print the integer
   
      if (x == 0){Serial.println("Doing nothing!");}
      if (x == 1){poweron();}
      if (x == 2){poweroff();} 
      if (x == 100){forward = true;}      
      if (x > 100 && x < 112){manualservo(x);}   
      if (x > 119 && x < 130){manualmotor(x);}
      if (x == 200){esc1.write(70); servo1.write(middlePosition);}    //Move forward at steady state
    }
     
  else if(howMany == 2){
    Serial.println("----Multi-byte data received----");    
    t1 = Wire.read();
    Serial.print("Byte 1: "); 
    Serial.println(t1);
    Serial.print("Byte 2: ");
    t2 = Wire.read();
    Serial.println(t2);
    Serial.println("--------------------------------");
  }
  else{
    Serial.println("Error! Commands must be 1 or 2 bytes.");
  }
}

void requestEvent(){
      if(taskComplete == false){
        Wire.write(251);
        Serial.println ("Sent back 251");
        return;
      }
      if(taskComplete == true){
        Wire.write(250); 
        Serial.println ("Sent back 250");
        return;
      }

      

  /*
  switch(lastcommand){
    case 230:
      Wire.write(batterylevel);
      break;     
  }
  */
}
