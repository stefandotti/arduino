#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#include<Wire.h>
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
void setup(){
  pixels.begin(); // This initializes the NeoPixel library.
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  //Serial.begin(9600);
}

int old_x, old_y, old_z;
int pos = 0, old_pos = -1;
int delayval = 100;
boolean full = false;
boolean stopped = true;

void loop(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  //Serial.print("AcX = "); Serial.print(AcX);
  //Serial.print(" | AcY = "); Serial.print(AcY);
  //Serial.print(" | AcZ = "); Serial.println(AcZ);

  int x = abs(AcX);
  int y = abs(AcY);
  int z = abs(AcZ);

  int r = 0, g = 0, b = 50;
  if (abs(x - old_x) > 10000 || abs(y - old_y) > 10000 || abs(z - old_z) > 10000) {
    delayval = 10;
    b = 255;
    full = true;
  } else if (abs(x - old_x) > 7000 || abs(y - old_y) > 7000 || abs(z - old_z) > 7000) {
    delayval = 50;
    b = 160;
  } else if (abs(x - old_x) > 4000 || abs(y - old_y) > 4000 || abs(z - old_z) > 4000) {
    delayval = 100;
    b = 100;
  } else if (abs(x - old_x) > 400 || abs(y - old_y) > 400 || abs(z - old_z) > 400) {
    delayval = 300;
    b = 50;
  } else {
    delayval = 400;
    b = 10;
    if (full) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        pixels.show();
      }
      delay(3000);
    }
    full = false;
  }

  if (old_pos > -1) {
    pixels.setPixelColor(old_pos, pixels.Color(0, 0, 10));
    pixels.show();
  }
  pixels.setPixelColor(pos, pixels.Color(r, g, b));
  pixels.show();
  delay(delayval);

  if (pos > NUMPIXELS) {
     pos = -1;
  }

  old_x = x;
  old_y = y;
  old_z = z;
  old_pos = pos++;
}

