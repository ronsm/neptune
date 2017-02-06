// From http://www.engineersgarage.com/contribution/arduino-based-ultrasonic-radar

#include <Servo.h> // include header file for servo library

Servo myservo; // create instance of servo class

const int trigPin = 10; // declare UDM trigger pin
const int echoPin = 11; // declare UDM echo pin
int radar_angle = 0, actual_radar_angle;
void setup() 
{
  Serial.begin(9600);             // initialize serial communication:
  pinMode(trigPin, OUTPUT); // trig pin as output
  pinMode(echoPin, INPUT); // echo pin as input
   myservo.attach(12);  // attach servo motor to pin 11
   myservo.write(90);   // rotate it to 90 deg first
}
void loop()
{
  long duration,distance_cm;
//increase radar angle from 0 to 180 deg in step of 30 deg
  for (radar_angle = 0; radar_angle <= 180; radar_angle += 30)
{ 
    myservo.write(radar_angle); // rotate motor    
    digitalWrite(trigPin, LOW); // apply trig as 0-1-0
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // for 10 us
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH); // get the pulse duration
// calculate the distance   
    distance_cm = microsecondsToCentimeters(duration);
if(distance_cm<200)
 {
         Serial.print("Object Detected at an angle = "); // print message
         actual_radar_angle = myservo.read(); // print motor actual angle
         Serial.print(actual_radar_angle);
         Serial.print(" deg and distance = ");
         Serial.print(distance_cm); // and object distance
         Serial.print(" cm");
         Serial.println();
      }
delay(500); // wait for half second
  }  
// now rotate motor reverse and do same as above  
for (radar_angle = 180; radar_angle >=0; radar_angle -= 30)
{ 
    myservo.write(radar_angle);    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);   
    distance_cm = microsecondsToCentimeters(duration);
    if(distance_cm<200)
    {
         Serial.print("Object Detected at an angle = ");
         actual_radar_angle = myservo.read();
         Serial.print(actual_radar_angle);
         Serial.print(" deg and distance = ");
         Serial.print(distance_cm);
         Serial.print(" cm");
         Serial.println();
      }
delay(500);
  }  
}
long microsecondsToCentimeters(long microseconds)
{
  return microseconds / 58;
}
 
