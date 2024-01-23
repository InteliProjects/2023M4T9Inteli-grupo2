#include "SCD.h"

#define LOADCELL_DATA_PIN 25
#define LOADCELL_CLOCK_PIN 33

SCD scd;

void setup() {
  // put your setup code here, to run once:
  scd.init();
  scd.connectToWifi("Inteli-COLLEGE", "QazWsx@123");
  scd.connectToHX711(LOADCELL_DATA_PIN, LOADCELL_CLOCK_PIN);
  delay(150);
  scd.disconnectWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  scd.showMensageOnDisplayMenu("Peso Atual", scd.getWeight(), "Kg", 3);
  scd.alerts();
}
