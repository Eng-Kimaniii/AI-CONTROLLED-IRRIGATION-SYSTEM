#include "DHT.h"

// ============================================================
// PIN DEFINITIONS
// ============================================================
#define DHTPIN        4    // DHT22 data pin
#define DHTTYPE       DHT22
#define SOIL_PIN      34   // Capacitive soil moisture sensor

// ============================================================
// CALIBRATION VALUES — update after soil sample calibration
// ============================================================
#define DRY_VALUE     3200  // raw ADC reading in completely dry soil
#define WET_VALUE     1100  // raw ADC reading in saturated soil

// ============================================================
// IRRIGATION THRESHOLD
// ============================================================
#define MOISTURE_THRESHOLD  40  // irrigate if soil moisture below 40%

// ============================================================
// SLEEP DURATION
// ============================================================
#define TEST_SLEEP_SECONDS   30   // use during testing
#define DEPLOY_SLEEP_SECONDS 600  // 10 minutes — use for deployment

// ============================================================
// OBJECTS
// ============================================================
DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// SENSOR READ FUNCTIONS
// ============================================================

float readTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("  [ERROR] Failed to read temperature from DHT22!");
    return -999;
  }
  return t;
}

float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("  [ERROR] Failed to read humidity from DHT22!");
    return -999;
  }
  return h;
}

int readSoilMoisture() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, DRY_VALUE, WET_VALUE, 0, 100);
  moisture = constrain(moisture, 0, 100);
  return moisture;
}

// ============================================================
// IRRIGATION DECISION
// ============================================================

bool shouldIrrigate(int moisture, float temp, float humidity) {
  // Basic threshold logic — will be replaced by Random Forest
  // model decision from Raspberry Pi in later integration phase
  if (moisture < MOISTURE_THRESHOLD) {
    return true;
  }
  return false;
}

// ============================================================
// PRINT SEPARATOR
// ============================================================

void printSeparator() {
  Serial.println("------------------------------------------");
}

// ============================================================
// SETUP — runs once every wake cycle
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(2000);  // wait for serial monitor to connect
  dht.begin();
  delay(500);   // let DHT22 stabilize

  printSeparator();
  Serial.println("ESP32 Irrigation Spoke Node — Awake");
  printSeparator();

  // --- READ DHT22 ---
  Serial.println("Reading DHT22...");
  float temperature = readTemperature();
  float humidity    = readHumidity();

  if (temperature != -999 && humidity != -999) {
    Serial.print("  Temperature : ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("  Humidity    : ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // --- READ SOIL MOISTURE ---
  Serial.println("Reading soil moisture...");
  int soilRaw      = analogRead(SOIL_PIN);
  int soilMoisture = readSoilMoisture();

  Serial.print("  Raw ADC     : ");
  Serial.println(soilRaw);
  Serial.print("  Moisture    : ");
  Serial.print(soilMoisture);
  Serial.println(" %");

  // --- IRRIGATION DECISION ---
  printSeparator();
  bool irrigate = shouldIrrigate(soilMoisture, temperature, humidity);

  if (irrigate) {
    Serial.println("DECISION: IRRIGATE");
    Serial.println("  Soil moisture below threshold!");

    // --- MOSFET ACTUATION PLACEHOLDER ---
    // Uncomment these lines once MOSFET + solenoid are connected:
    // digitalWrite(26, HIGH);  // open solenoid valve
    // digitalWrite(27, HIGH);  // start pump
    // delay(300000);           // run for 5 minutes
    // digitalWrite(27, LOW);   // stop pump
    // digitalWrite(26, LOW);   // close solenoid valve
    // Serial.println("  Irrigation cycle complete.");

  } else {
    Serial.println("DECISION: SKIP — soil moisture sufficient");
  }

  // --- UDP SEND PLACEHOLDER ---
  // Uncomment once WiFi + Raspberry Pi UDP listener are set up:
  // sendUDP(temperature, humidity, soilMoisture, irrigate);

  // --- SLEEP ---
  printSeparator();
  Serial.print("Sleeping for ");
  Serial.print(TEST_SLEEP_SECONDS);
  Serial.println(" seconds...");
  Serial.println("(Change to DEPLOY_SLEEP_SECONDS for field use)");
  printSeparator();
  Serial.flush();

  esp_sleep_enable_timer_wakeup(TEST_SLEEP_SECONDS * 1000000ULL);
  esp_deep_sleep_start();
}

// ============================================================
// LOOP — never runs, ESP32 restarts from setup() after wake
// ============================================================

void loop() {}