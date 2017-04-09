#include <Wire.h> // I2C 
#define addr 0x0D // I2C
#define how_units_in_range 180.0

int x_min, x_max, y_min, y_max, z_min, z_max; 
float range_x, range_y, range_z;
int count;

void setup() {
  x_min = y_min = z_min = 32767;
  x_max = y_max = z_max = -32768;
  count = 0;

  Serial.begin(9600);
  Wire.begin();

  Wire.beginTransmission(addr); 
  Wire.write(0x09); 
  Wire.write(0x01); 
  Wire.endTransmission();
}

void loop() {
  int x, y, z; //triple axis data
  
  Wire.beginTransmission(addr);
  Wire.write(0x01);             
  Wire.requestFrom(addr, 6);

  ++count;

  if (count < 3){
    x_min = y_min = z_min = 32767;
    x_max = y_max = z_max = -32768;
    Serial.println("----------------->>>>>>>>initial");
  }

  
  while (Wire.available()){   
    x = Wire.read() << 8; 
    x |= Wire.read(); 

    if (x_min > x){ x_min = x; Serial.print("x_min udate = "); Serial.println(x); }
    if (x_max < x){ x_max = x; Serial.print("x_max udate = "); Serial.println(x); }

    y = Wire.read() << 8; 
    y |= Wire.read(); 

    if (y_min > y){ y_min = y; Serial.print("y_min udate = "); Serial.println(y); }
    if (y_max < y){ y_max = y; Serial.print("y_max udate = "); Serial.println(y); }

    z = Wire.read() << 8; 
    z |= Wire.read(); 

    if (z_min > z){ z_min = z; Serial.print("z_min udate = "); Serial.println(z); }
    if (z_max < x){ z_max = z; Serial.print("z_max udate = "); Serial.println(z); }
  }
    
  
  float heading = atan2(y, x);

  float declinationAngle = 0.0404;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;

  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;

  float headingDegrees = heading * 180/M_PI; 
  Serial.print("Heading: ");
  Serial.println(headingDegrees);

  
  Wire.endTransmission();
  
  range_x = x_max - x_min; 
  range_y = y_max - y_min;
  range_z = z_max - z_min;

  float position_x = x_max - x;
  float position_y = y_max - y;
  float position_z = z_max - z;
 
  float real_x = (position_x * how_units_in_range) / range_x;
  float real_y = (position_y * how_units_in_range) / range_y;
  float real_z = (position_z * how_units_in_range) / range_z;

  Serial.print(" range_x: ");
  Serial.print(range_x);
  Serial.print("\t\trange_y: ");
  Serial.print(range_y);
  Serial.print("\t\trange_z: ");
  Serial.print(range_z);
  Serial.println();

  Serial.print("   pos_x: ");
  Serial.print(position_x);
  Serial.print("\t\t  pos_y: ");
  Serial.print(position_y);
  Serial.print("\t\t  pos_z: ");
  Serial.print(position_z);
  Serial.println();
/*
  Serial.print("       x: ");
  Serial.print(x);
  Serial.print("\t\t      y: ");
  Serial.print(y);
  Serial.print("\t\t      z: ");
  Serial.print(z);
  Serial.println();
*/
  Serial.println();
  Serial.println("------------------------------degrees-----------------------------------");
  
  Serial.print(" x: ");
  Serial.print(real_x);
  Serial.print("\t\t      y: ");
  Serial.print(real_y);
  Serial.print("\t\t      z: ");
  Serial.print(real_z);
  Serial.println();

  Serial.println("________________________________________________________________________");

//Serial.println();
  delay(1000);
}
