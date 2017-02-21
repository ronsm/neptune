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

int incomingByte = 0;                     // for incoming serial data
int on = 1;
int off = 2;

Servo esc1;
Servo esc2;
Servo servo1;

void setup() {
        
          Wire.begin(3);                // join i2c bus with address #3
          Wire.onReceive(receiveEvent); // register event
          Serial.begin(9600);           // opens serial port, sets data rate to 9600 bps
          servo1.attach(3);
          esc1.attach(10);
          esc2.attach(6);

          //Initialisation of speed controlers and reset to default values
          esc1.write(20);
          esc2.write(20);
          servo1.write(90);              // Rudder at middle position
          delay(1000);
}

void loop() {
  
  delay(10);

        // Debug receive from serial monitor
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();
                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte);
        }

}


// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while (1 < Wire.available())           // loop through all but the last
  {
    char c = Wire.read();                // receive byte as a character
    Serial.print(c);                     // print the character
  }
  int x = Wire.read();                   // receive byte as an integer
  Serial.println(x);                     // print the integer
  
          if (x == on)
        {
          Serial.print("Power ON");
          Serial.print("\n");
          esc2.write(80);
        }
        
          if (x == off)
        {
          Serial.print("Power OFF");
          Serial.print("\n");
          esc2.write(20);
          esc2.write(20);
          //servo1.write(90);       
        }

          //Lookup for servo position
          if (x > 100 && x < 112){
          Serial.print("Servo");
          Serial.print("\n");
            if (x == 101){servo1.write(0);}
            if (x == 102){servo1.write(20);}
            if (x == 103){servo1.write(40);}
            if (x == 104){servo1.write(60);}
            if (x == 105){servo1.write(80);}
            if (x == 106){servo1.write(90);}
            if (x == 107){servo1.write(100);}
            if (x == 108){servo1.write(120);}
            if (x == 109){servo1.write(140);}
            if (x == 110){servo1.write(160);}
            if (x == 110){servo1.write(180);}       
          }
          
          //Lookup for rear motor throttle amount
          if (x > 119 && x < 130){
            if (x == 120){esc1.write(20);}
            if (x == 121){esc1.write(40);}
            if (x == 122){esc1.write(60);}
            if (x == 123){esc1.write(80);}
            if (x == 124){esc1.write(90);}
            if (x == 125){esc1.write(100);}
            if (x == 126){esc1.write(120);}
            if (x == 127){esc1.write(140);}
            if (x == 128){esc1.write(160);}
            if (x == 129){esc1.write(180);}       
          }

}



