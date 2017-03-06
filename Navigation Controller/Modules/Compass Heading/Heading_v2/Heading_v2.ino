#include "quaternionFilters.h"
#include "MPU9250.h"
#include <mpu9250.h>

int intPin = 12;  // These can be changed, 2 and 3 are the Arduinos ext int pins
int myLed  = 13;  // Set up pin 13 led for toggling

float Pi = 3.141593;

int mxcalmax = 0;
int mxcalmin = 1000;
int mycalmax = 0;
int mycalmin = 1000;
int MagX;
int MagY;

MPU9250 myIMU;

void setup() {
  Wire.begin();
  Serial.begin(38400);

  pinMode(intPin, INPUT);
  digitalWrite(intPin, LOW);
  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH);
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
    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values
    myIMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    myIMU.magbias[0] = xmbias();
    // User environmental x-axis correction in milliGauss TODO axis??
    myIMU.magbias[1] = ymbias();
    // User environmental x-axis correction in milliGauss
    myIMU.magbias[2] = 0;

    
  } 
  myIMU.updateTime();

  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx*DEG_TO_RAD, myIMU.gy*DEG_TO_RAD, myIMU.gz*DEG_TO_RAD, myIMU.my,myIMU.mx, myIMU.mz, myIMU.deltat);

    myIMU.delt_t = millis() - myIMU.count;

    if (myIMU.delt_t > 500)
    {
     
      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;

  myIMU.mx = (float)myIMU.magCount[0]*myIMU.mRes*myIMU.magCalibration[0] - myIMU.magbias[0];
  myIMU.my = (float)myIMU.magCount[1]*myIMU.mRes*myIMU.magCalibration[1] - myIMU.magbias[1];
  myIMU.mz = (float)myIMU.magCount[2]*myIMU.mRes*myIMU.magCalibration[2] - myIMU.magbias[2];

  Serial.print("mx = "); Serial.print( (int)myIMU.mx );
  Serial.print(" my = "); Serial.print( (int)myIMU.my );
  Serial.println(" mG");

  Serial.print("X Bias: ");
  Serial.println (xmbias());

  Serial.print("Y Bias: ");
  Serial.println (ymbias());

  Serial.print("Heading: ");
  Serial.println(getHeading());
    } 
}
double getHeading(){

  if (myIMU.my == 0){
    if (myIMU.mx > 0){
      return 0;
    }
    else{
      return 180;
    }
  }
  else{
    if (myIMU.my > 0){
      return (90 - (atan(myIMU.mx/myIMU.my)*180/Pi));
    }
    else{
      return (270 - (atan(myIMU.mx/myIMU.my)*180/Pi));
    }
  }
} 

int xmbias (){

  MagX = (float)myIMU.magCount[0]*myIMU.mRes*myIMU.magCalibration[0];
  count++;

  if (mxcalmax < MagX){
    mxcalmax = MagX;
  }
  if (mxcalmin > MagX){
    mxcalmin = MagX;
  }

//  if (count >= 999){
//    mxcalmax = myIMU.mx;
//    mxcalmax = myIMU.mx;
//    count = 0;
//  }
  
  return (mxcalmax+mxcalmin)/2;
}

int ymbias (){

  MagY = (float)myIMU.magCount[1]*myIMU.mRes*myIMU.magCalibration[1];
  count ++;
  
  if (mycalmax < MagY){
    mycalmax = MagY;
  }
  if (mycalmin > MagY){
    mycalmin = MagY;
  }

//  if (count >= 999){
//    mycalmax = myIMU.my;
//    mycalmax = myIMU.my;
//    count = 0;
//  }

  return (mycalmax+mycalmin)/2;
}
