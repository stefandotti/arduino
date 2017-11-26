#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8

#define STOPPED 0
#define ACC     1
#define RUNNING 2
#define DECC    3

#define DELAY   100
#define BASE    0
#define DELTA   4000

#define XOFFSET 0
#define YOFFSET -500
#define ZOFFSET 1800

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#include<Wire.h>
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
}

int old_x = 0, old_y = 0, old_z = 0;
int running_mode = 0;
int speed = 0;
int l = 0, speedi = 0, speeddelta = 0;
boolean cc = false;

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  AcX -= XOFFSET;
  AcY -= YOFFSET;
  AcZ -= ZOFFSET;

  //  Serial.print("AcX = "); Serial.print(AcX);
  //  Serial.print(" | AcY = "); Serial.print(AcY);
  //  Serial.print(" | AcZ = "); Serial.println(AcZ);
  //
  //  Serial.print("mode: "); Serial.println(running_mode);

  if (BASE == 0) {
    // x, z is sideways, y is for-backward
    speed = AcY;
    if (abs(old_y - AcY) > DELTA) {
      if (old_y < AcY) {
        running_mode = ACC;
      } else {
        running_mode = DECC;
      }
    } else if (speed > 500) {
      running_mode = RUNNING;
    }
  } else if (BASE == 1) {
    // y, z is sideways, x is for-backward
  } else if (BASE == 2) {
    // z, y is sideways, x is for-backward
  }

  Serial.println(speed);

  if (speed < 500) {
    speed = 0;
    running_mode = STOPPED;
  }

  // set the base color
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 55));
  }

  // set color according to mode
  {
    if (running_mode == STOPPED) {
      // stopped
      pixels.setPixelColor(l, pixels.Color(10, 0, 0));
    } else if (running_mode == ACC) {
      // accelerating
      pixels.setPixelColor(l, pixels.Color(0, 0, 255));
    } else if (running_mode == RUNNING) {
      // running
      pixels.setPixelColor(l, pixels.Color(0, 255, 0));
    } else if (running_mode == DECC) {
      // deccelerating
      pixels.setPixelColor(l, pixels.Color(255, 0, 0));
    } else {
      // unknown
      pixels.setPixelColor(l, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }
  
  speedi++;
  int a = 10 - (speed / DELAY);
  if (a < 0) a = 1;
  if (speedi % a == 0) {
    l++;
  }

  if (l >= NUMPIXELS) l = 0;

  old_x = AcX;
  old_y = AcY;
  old_z = AcZ;

  delay(DELAY);
}

