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
#define DELTA   1
#define AVG     1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(pinSwitch, INPUT); // reed
  //Serial.begin(9600);
}

int old_x = 0, old_y = 0, old_z = 0;
int running_mode = 0;
int speed = 0, currentSpeed = 0;
int old_l = 0, old_c = 0, c = 0, l = 0, speedi = 0, speeddelta = 0, j = 0;
boolean cc = false;
int avgspeed;
int StatoSwitch = 0;
boolean stat = false;
int countStopped = 0;
int wheelCount = 0;

void loop() {
  StatoSwitch = digitalRead(pinSwitch);

  if (!stat && StatoSwitch == HIGH) {
    c++;
    wheelCount++;
    stat = true;
  } else if (StatoSwitch == LOW) {
    stat = false;
  }

  if (j >= NUMPIXELS * 2) {
    j = 0;

    //    Serial.print("wheel count: "); Serial.println(wheelCount);
    //    Serial.println((double)(wheelCount * 10) / ((double)NUMPIXELS * 2));
    speed = ceil((double)(wheelCount * 10) / ((double)NUMPIXELS * 2));
    wheelCount = 0;
    if (c == old_c) {
      // no speed change
      c = 0;
      speed = 0;
      //      Serial.println("reset speed to zero");
    }

    //    Serial.print("reed count: "); Serial.println(c);
    // x, z is sideways, y is for-backward
    if (speed - old_y > 0) {
      running_mode = ACC;
    } else if (speed == 0 && (old_y > 0 || countStopped > 0)) {
      if (countStopped++ > 3) {
        running_mode = DECC;
        countStopped = 0;
      }
    } else if (speed > 0) {
      running_mode = RUNNING;
    } else if (speed == 0) {
      running_mode = STOPPED;
    }

    //Serial.print(running_mode); Serial.print(" - "); Serial.println(speed);

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
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
      }
    } else {
      // unknown
      pixels.setPixelColor(l, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }

  old_l = l;
  speedi++;
  int a = 20 / speed;
  if (a < 0) a = 1;
  if (speedi % a == 0) {
    l++;
  }

  if (l >= NUMPIXELS) l = 0;

  delay(DELAY);
}

