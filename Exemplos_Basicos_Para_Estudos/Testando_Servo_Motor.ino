#include "Servo.h" 
 
Servo servoD1; 

void setup() {
  // put your setup code here, to run once:
  servoD1.attach(D1);
}

void loop() {
  // put your main code here, to run repeatedly:
  servoD1.write(5);
  delay(1000);
  servoD1.write(90);
  delay(1000);
  servoD1.write(180);
  delay(1000);
  servoD1.write(90);
  delay(1000);
}
