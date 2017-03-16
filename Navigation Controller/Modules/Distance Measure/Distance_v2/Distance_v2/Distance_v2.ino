#include "quaternionFilters.h"
#include "MPU9250.h"
#include <mpu9250.h>
#include <math.h>

float Pi = 3.141593;

double prevacctime = 0;
double totaldistance;
double accelbias=0;
boolean accelbiascheck = false;
int count = 0;

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
  myIMU.updateTime();

  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx*DEG_TO_RAD, myIMU.gy*DEG_TO_RAD, myIMU.gz*DEG_TO_RAD, myIMU.my,myIMU.mx, myIMU.mz, myIMU.deltat);

  myIMU.delt_t = millis() - myIMU.count;

  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {  
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();
    myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes;
  } 

    if (myIMU.delt_t > 100){
      getDistance();
      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;
    } 
}

void getDistance (){

  double distance;
  double curacctime = millis(); 
  double accelTime = (curacctime - prevacctime)/1000;
  Serial.print("Time: "); Serial.print(millis()/1000); Serial.println ("s");
  
  myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes - accelbias;


    if (myIMU.ay < 0){
      myIMU.ay = -1 * myIMU.ay;
    }

    if (myIMU.ay < 0.075){
      myIMU.ay = 0.00;
    }
    Serial.print("Y-acceleration: "); Serial.print(myIMU.ay);
    Serial.println(" g ");
  
    Serial.print("Distance between "); Serial.print(prevacctime); Serial.print(" - "); Serial.print(curacctime); Serial.print("ms: ");
    distance = 0.5 * myIMU.ay * 9.80665 * accelTime * accelTime;
    Serial.print(distance); Serial.println("m");
    
    Serial.print("Total distance: ");
    totaldistance = distance + totaldistance;
    Serial.print(totaldistance); Serial.println("m");
  
    prevacctime = curacctime;
}



