#include "quaternionFilters.h"
#include "MPU9250.h"
#include <mpu9250.h>
#include <math.h>

float Pi = 3.141593;

MPU9250 myIMU;

void setup() {
  Wire.begin();
  Serial.begin(38400);
  
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 "); Serial.print("I AM "); Serial.print(c, HEX);
  Serial.print(" I should be "); Serial.println(0x71, HEX);

  if (c == 0x71) // WHO_AM_I should always be 0x68
  {
    Serial.println("MPU9250 is online...");

    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.SelfTest);
    
    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);

    myIMU.initMPU9250();
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);

    myIMU.initAK8963(myIMU.magCalibration);
  }
  else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {  
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0]*myIMU.aRes; // - accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes; // - accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2]*myIMU.aRes; // - accelBias[2];
  } 
  myIMU.updateTime();

  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx*DEG_TO_RAD, myIMU.gy*DEG_TO_RAD, myIMU.gz*DEG_TO_RAD, myIMU.my,myIMU.mx, myIMU.mz, myIMU.deltat);

    myIMU.delt_t = millis() - myIMU.count;

    if (myIMU.delt_t > 1000)
    {

      Serial.print("Distance: "); Serial.println(getDistance());
      
      myIMU.tempCount = myIMU.readTempData();  // Read the adc values
      // Temperature in degrees Centigrade
      myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
      // Print temperature in degrees Centigrade
      Serial.print("Temperature is ");  Serial.print(myIMU.temperature, 1);
      Serial.println(" degrees C");
     
      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;
    } 
}

double getDistance (){

  double distance;
  double accelTime = millis() / 1000; 
  Serial.print("Time (in s): "); Serial.println(accelTime);
  
  myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes;
  
  if (myIMU.ay < 0){
    myIMU.ay = myIMU.ay * (-1);
  }
  
  Serial.print("Y-acceleration: "); Serial.print(myIMU.ay);
  Serial.println(" ug ");
  
  distance = 0.5 * myIMU.ay * 9.80665 / 1000 * accelTime * accelTime + distance;
  return distance;  
}
 
