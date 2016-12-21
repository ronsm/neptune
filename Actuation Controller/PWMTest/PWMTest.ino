/*
 PWMTest

 This program sweeps from 0% to 100% duty cycle on the arduino
 using the built in analogwrite() function.

 The circuit:
 * PWM output attached to pin 9 of the arduino

 Created 10 Dec 2016
 By Sebastian Smith

 */

int pwm    = 9;   // LED connected to digital pin 9
int delay1 = 30;  // Speed at which sweep is made (increase for slower)

void setup() {
  // No setup needed for this program
}

void loop() {
  // Increase duty cycle in increments of 5
  for (int pwmValue = 0 ; pwmValue <= 255; pwmValue += 5) {
    analogWrite(pwm, pwmValue);
    delay(delay1);
  }

  // Decrease duty cycle in increments of 5
  for (int pwmValue = 255 ; pwmValue >= 0; pwmValue -= 5) {
    analogWrite(pwm, pwmValue);
    delay(delay1);
  }
}
