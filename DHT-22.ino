#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(2000);
  dht.begin();
}

void loop() {
  delay(3000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT22!");
    return;
  }

  Serial.print("Temp: "); Serial.print(t); Serial.print(" C  |  ");
  Serial.print("Humidity: "); Serial.print(h); Serial.println(" %");
}
