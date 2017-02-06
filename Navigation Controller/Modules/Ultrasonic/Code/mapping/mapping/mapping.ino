void setup() {
  // put your setup code here, to run once:

}

int   maparray [100] [100] = 0;     //map of obstacles
int   prevX [20]=0;                 //used to store previous coordinates 
int   prevY [20]=0; 

int   angle;                        //servo's angle
int   pos;                          //servo's position (based on angle)
int   Xaxis;                        //current obstacle coordinates
int   Yaxis;

float cmdistanc;                    //distance from obstac;e
long  microsec = ultrasonic.timing();
  
void loop() {
  //  put your main code here, to run repeatedly:
  
  //  angle = radar_angle;                                            //get servo angle
  //  pos = angle/10;                                                 //get servo position
  //  maparray [prevX[pos]] [prevY[pos]] = 0;                         //errase previous obstacle at given position
  //  cmdistance = ultrasonic.CalcDistance(microsec,Ultrasonic::CM);  //get obstacle distance
  //  Xaxis = (cmdistance * sin(angle) + 50)/2.5;                     //get x coordinate according to trigonometry (+50 for placement along the axis)
  //  Yaxis = cmdistance * cos(angle)/ 2.5;                           //get y coordinate ''
  //  maparray [Xaxis] [Yaxis] = 1;                                   //place obstacle in map array
  //  prevX [pos]= Xaxis;                                             //store coordinates in array according to servo position
  //  prevY [pos]= Yaxis;
}
