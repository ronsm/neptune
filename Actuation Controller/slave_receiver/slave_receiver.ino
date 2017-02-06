#include <Wire.h>
#include <Servo.h>

int level1 = 2;
int level2 = 1;

Servo esc1;
Servo esc2;
Servo rudder;
Servo radar;


void setup()
{
  Wire.begin(3);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output

  esc1.attach(3);
  esc2.attach(6);

  esc1.write(0);
  esc2.write(20);
  delay(1000);

}

void loop()
{
  delay(100);
  
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while (1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
  
          if (x == level1)
        {
          Serial.print("Motors OFF");
          Serial.print("\n");
          esc2.write(20);
        }
        
          if (x == level2)
        {
          Serial.print("Motors ON");
          Serial.print("\n");
          esc2.write(100);
        }
  
}
