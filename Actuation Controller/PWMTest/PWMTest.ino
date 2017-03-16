/*
 PWMTest

 This program sweeps from 0% to 100% duty cycle on the arduino
 using the built in analogwrite() function.

 The circuit:
 * PWM output attached to pin 9 of the arduino

 Created 10 Dec 2016
 By Sebastian Smith

 */
 
 #include <Servo.h>

int l1 = 49;
int l2 = 50;
int l3 = 51;
int l4 = 52;
int l5 = 53;
int l6 = 54;
int l7 = 55;
int l8 = 56;
int l9 = 57;

int incomingByte = 0;      
Servo esc;

int done = 0;

void setup() {
  Serial.begin(9600);
  esc.attach(6);
  esc.write(20);
  delay(3000);
  
}

void loop() {
  
  delay(10);


/*
          // Debug receive from serial monitor
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();
                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte);
                done = 1;
        }
        
        if (incomingByte == l1){
          if (done == 1){
             Serial.println("PWM: 0");
             esc.write(0);
             done = 0;
          }
        }
        
        
       if (incomingByte == l2){
          if (done == 1){
             Serial.println("PWM: 20");
             esc.write(20);
             done = 0;
          }
        }
        
       if (incomingByte == l3){
          if (done == 1){
             Serial.println("PWM: 100");
             esc.write(90);
             done = 0;
          }
        }
        
       if (incomingByte == l4){
          if (done == 1){
             Serial.println("PWM: 254");
             esc.write(110);
             done = 0;
          }
        }

*/

esc.write(90);

}
