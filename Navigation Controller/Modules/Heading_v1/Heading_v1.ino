#include "quaternionFilters.h"
#include "MPU9250.h"
#include <mpu9250.h>
#include <SPI.h>
#include <Wire.h>

float Pi = 3.141593;

MPU9250 IMU;

void setup() {
  Wire.begin();
  // TWBR = 12;  // 400 kbit/sec I2C speed
  Serial.begin(57600);

  // Set up the interrupt pin, its set as active high, push-pull

  // Read the WHO_AM_I register, this is a good test of communication
  byte c = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 "); Serial.print("I AM "); Serial.print(c, HEX);
  Serial.print(" I should be "); Serial.println(0x71, HEX);

  if (c == 0x71) // WHO_AM_I should always be 0x68
  {
    Serial.println("MPU9250 is online...");

    // Start by performing self test and reporting values
    IMU.MPU9250SelfTest(IMU.SelfTest);
    
    // Calibrate gyro and accelerometers, load biases in bias registers
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);

    IMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature

    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = IMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);

    // Get magnetometer calibration from AK8963 ROM
    IMU.initAK8963(IMU.magCalibration);
    // Initialize device for active mode read of magnetometer

  } // if (c == 0x71)
  else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
}

void loop() {
  // put your main code here, to run repeatedly:

      Serial.println(getHeading());
      delay (1000);
     // if (IMU.delt_t > 500)

  
}
double getHeading(){

  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {  
    IMU.readMagData(IMU.magCount);  // Read the x/y/z adc values
    IMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    IMU.magbias[0] = 0;
    // User environmental x-axis correction in milliGauss TODO axis??
    IMU.magbias[1] = 0;
    // User environmental x-axis correction in milliGauss
    IMU.magbias[2] = 0;

    IMU.mx = (float)IMU.magCount[0]*IMU.mRes*IMU.magCalibration[0] -
               IMU.magbias[0];
    IMU.my = (float)IMU.magCount[1]*IMU.mRes*IMU.magCalibration[1] -
               IMU.magbias[1];
    IMU.mz = (float)IMU.magCount[2]*IMU.mRes*IMU.magCalibration[2] -
               IMU.magbias[2];
  } 
  IMU.updateTime();

  MahonyQuaternionUpdate(IMU.ax, IMU.ay, IMU.az, IMU.gx*DEG_TO_RAD,
                         IMU.gy*DEG_TO_RAD, IMU.gz*DEG_TO_RAD, IMU.my,
                         IMU.mx, IMU.mz, IMU.deltat);

  
    // Serial print and/or display at 0.5 s rate independent of data rates
    IMU.delt_t = millis() - IMU.count;

    // update LCD once per half-second independent of read rate
    if (IMU.delt_t > 500)
    {
        Serial.print("mx = "); Serial.print( (int)IMU.mx );
        Serial.print(" my = "); Serial.print( (int)IMU.my );
        Serial.print(" mz = "); Serial.print( (int)IMU.mz );
        Serial.println(" mG");
     
      IMU.count = millis();
      IMU.sumCount = 0;
      IMU.sum = 0;
  
  if (IMU.my == 0){
    if (IMU.mx > 0){
      return 0;
    }
    else{
      return 180;
    }
  }
  else{
    if (IMU.my > 0){
      return (90 - (atan(IMU.mx/IMU.my)*180/Pi));
    }
    else{
      return (270 - (atan(IMU.mx/IMU.my)*180/Pi));
    }
  }
    } 
}
