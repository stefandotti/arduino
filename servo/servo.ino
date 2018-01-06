#include <Servo.h>
#define pin 3
Servo servo;
int pos = 90;
int d = 1;
void setup() {
  // put your setup code here, to run once:
  servo.attach(3);
  servo.write(pos);
  Serial.begin(9600);
}

void loop() {
  pos += d;
  if (pos <= 0 || pos >= 180) {
    d *= -1;
    digitalWrite(13, d > 0 ? HIGH : LOW);
  }
  servo.write(pos);
  delay(15);
}
