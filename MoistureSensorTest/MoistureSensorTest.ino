#include <ESP8266WiFi.h>

void setup() {
    Serial.begin(115200);
}
void loop() {
    delay(200);
    Serial.println(analogRead(A0));
}