int incomingByte = 0;   // for incoming serial data
int level1 = 49;
int level2 = 50;
int level3 = 51;
int level4 = 52;
int level5 = 53;
int level6 = 54;
int level7 = 55;
int level8 = 56;
int level9 = 57;

int outpin    = 3;

void setup() {
        Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
}

void loop() {
  
  delay(10);

        // send data only when you receive data:
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();

                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte);
        }
        
        if (incomingByte == level1)
        {
          Serial.print("Speed Level 1");
          Serial.print("\n");
          analogWrite(outpin, 0);
        }

        if (incomingByte == level2)
        {
          Serial.print("Speed Level 2");
          Serial.print("\n");
          analogWrite(outpin, 32);
        }

        if (incomingByte == level3)
        {
          Serial.print("Speed Level 3");
          Serial.print("\n");
          analogWrite(outpin, 64);
        }

        if (incomingByte == level4)
        {
          Serial.print("Speed Level 4");
          Serial.print("\n");
          analogWrite(outpin, 96);
        }

        if (incomingByte == level5)
        {
          Serial.print("Speed Level 5");
          Serial.print("\n");
          analogWrite(outpin, 128);
        }

        if (incomingByte == level6)
        {
          Serial.print("Speed Level 6");
          Serial.print("\n");
          analogWrite(outpin, 160);
        }

        if (incomingByte == level7)
        {
          Serial.print("Speed Level 7");
          Serial.print("\n");
          analogWrite(outpin, 192);
        }

        if (incomingByte == level8)
        {
          Serial.print("Speed Level 8");
          Serial.print("\n");
          analogWrite(outpin, 224);
        }

        if (incomingByte == level9)
        {
          Serial.print("Speed Level 9");
          Serial.print("\n");
          analogWrite(outpin, 255);
        }

}



