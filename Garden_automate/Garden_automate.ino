#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <cstdlib>

#include "network.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial){
    delay(10);
  }
  Network::establishConnection();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(WiFi.localIP());
  delay(5000);
  Network::loop();
}
