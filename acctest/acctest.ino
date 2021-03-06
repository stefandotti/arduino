#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8

#define pinSwitch     3 //Pin Reed

#define STOPPED 0
#define ACC     1
#define RUNNING 2
#define DECC    3

#define DELAY   20
#define BASE    0
#define DELTA   2

#define XOFFSET 0
#define YOFFSET -500
#define ZOFFSET 1800

#define AVG     1

int level = 300;

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
  pinMode(pinSwitch, INPUT); // reed 
  Serial.begin(9600);
  readmpu();
  level = AcY + 100;
}

int old_x = 0, old_y = 0, old_z = 0;
int running_mode = 0;
int speed = 0, currentSpeed = 0;
int old_l = 0, old_c = 0, c = 0, l = 0, speedi = 0, speeddelta = 0, j = 0;
boolean cc = false;
int avgspeed;
int StatoSwitch = 0;
boolean stat = false;

void loop() {
  readmpu();
  avgspeed += AcY;
  StatoSwitch = digitalRead(pinSwitch);
  
  //  Serial.print("AcX = "); Serial.print(AcX);
  //  Serial.print(" | AcY = "); Serial.print(AcY);
  //  Serial.print(" | AcZ = "); Serial.println(AcZ);
  //
  //  Serial.print("mode: "); Serial.println(running_mode);

  if (!stat && StatoSwitch == HIGH) {
    c++;
    stat = true;
  } else if (StatoSwitch == LOW) {
    stat = false;
  }

  if (j >= NUMPIXELS) {
    j = 0;
    Serial.print("reed count: "); Serial.println(c);
    //speed = avgspeed / NUMPIXELS;
    speed = c / NUMPIXELS;
    avgspeed = 0;
    if (BASE == 0) {
      // x, z is sideways, y is for-backward
      if (speed > level && abs(old_y - speed) > DELTA) {
        if (old_y < speed) {
          running_mode = ACC;
        } else {
          running_mode = DECC;
        }
      } else if (c > 0) {
        running_mode = RUNNING;
      } else if (c == 0) {
        running_mode = STOPPED;
      }
    } else if (BASE == 1) {
      // y, z is sideways, x is for-backward
    } else if (BASE == 2) {
      // z, y is sideways, x is for-backward
    }

    Serial.print(running_mode); Serial.print(" - "); Serial.print(level); Serial.print(" - "); Serial.println(speed);
  
    //old_y = speed;
    if (c == old_c) { c = 0; }
    currentSpeed = c / NUMPIXELS;
    old_y = speed;
    old_c = c;
  }
  j++;

  if (old_l > 0) {
    pixels.setPixelColor(old_l, pixels.Color(0, 0, 50));
  }

  // set color according to mode
  {
    if (running_mode == STOPPED) {
      // stopped
      pixels.setPixelColor(l, pixels.Color(0, 0, 50));
    } else if (running_mode == ACC) {
      // accelerating
      pixels.setPixelColor(l, pixels.Color(255, 255, 255));
    } else if (running_mode == RUNNING) {
      // running
      pixels.setPixelColor(l, pixels.Color(0, 0, 255));
    } else if (running_mode == DECC) {
      // deccelerating
      pixels.setPixelColor(l, pixels.Color(255, 0, 0));
    } else {
      // unknown
      pixels.setPixelColor(l, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }

  old_l = l;
  speedi++;
  int a = 10 - currentSpeed;
  if (a < 0) a = 1;
  if (speedi % a == 0) {
    l++;
  }

  if (l >= NUMPIXELS) l = 0;
}

void readmpu() {
  AcX = 0; AcY = 0; AcZ = 0; Tmp = 0; GyX = 0; GyY = 0; GyZ = 0;
  for(int n = 0; n < AVG; n++) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
    AcX += Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY += Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ += Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp += Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX += Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY += Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ += Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    delay(DELAY / AVG);
  }
  AcX /= AVG; AcY /= AVG; AcZ /= AVG; Tmp /= AVG; GyX /= AVG; GyY /= AVG; GyZ /= AVG;
}

