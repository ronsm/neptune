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
 * @version    1.0
 * @link       https://github.com/ronsm/neptune
 * 
 * ATTRIBUTIONS: This project uses and derives open source code and packages from
 * various authors, which are attributed here where possible.
 *    1) REF
 */

int incomingByte = 0;   // for incoming serial data
int level1 = 49;
int level2 = 50;
int level3 = 51;
int level4 = 52;
int level5 = 53;
int level6 = 54;
int level7 = 55;
int level8 = 56;
int level9 = 57;

#include <Servo.h>

//Servo esc1;
Servo esc2;

void setup() {
        Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
          //esc1.attach(3);
          esc2.attach(6);

          //esc1.write(0);
          esc2.write(20);
          delay(1000);
}

void loop() {
  
  delay(10);

        // send data only when you receive data:
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();

                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte);
        }
        
        if (incomingByte == level1)
        {
          Serial.print("Speed Level 1");
          Serial.print("\n");
          //analogWrite(outpin, 1);
          esc2.write(1);
        }

        if (incomingByte == level2)
        {
          Serial.print("Speed Level 2");
          Serial.print("\n");
          //analogWrite(outpin, 32);
          esc2.write(20);
        }

        if (incomingByte == level3)
        {
          Serial.print("Speed Level 3");
          Serial.print("\n");
          //analogWrite(outpin, 64);
          esc2.write(40);
        }

        if (incomingByte == level4)
        {
          Serial.print("Speed Level 4");
          Serial.print("\n");
          //analogWrite(outpin, 96);
          esc2.write(60);
        }

        if (incomingByte == level5)
        {
          Serial.print("Speed Level 5");
          Serial.print("\n");
          //analogWrite(outpin, 128);
          esc2.write(80);
        }

        if (incomingByte == level6)
        {
          Serial.print("Speed Level 6");
          Serial.print("\n");
          //analogWrite(outpin, 160);
          esc2.write(100);
        }

        if (incomingByte == level7)
        {
          Serial.print("Speed Level 7");
          Serial.print("\n");
          //analogWrite(outpin, 192);
          esc2.write(120);
        }

        if (incomingByte == level8)
        {
          Serial.print("Speed Level 8");
          Serial.print("\n");
          //analogWrite(outpin, 224);
          esc2.write(140);
        }

        if (incomingByte == level9)
        {
          Serial.print("Speed Level 9");
          Serial.print("\n");
          //analogWrite(outpin, 255);
          esc2.write(179);
        }

}



