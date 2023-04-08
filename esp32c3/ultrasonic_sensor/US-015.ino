#include <NewPing.h>

const int TRIGGER_PIN = 6; // <YOUR TRIGGER PIN NUMBER>
const int ECHO_PIN = 7; // <YOUR ECHO PIN NUMBER>
NewPing sonar(TRIGGER_PIN, ECHO_PIN);

void setup() {
  Serial.begin(115200);
}

void loop() {
  int distance = sonar.ping_cm();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(500);
}

