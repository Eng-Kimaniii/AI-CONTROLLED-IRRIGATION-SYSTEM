#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>   // ✅ REQUIRED for notifications

#define SENSOR_PIN 34

// UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Client Disconnected");

   
    BLEDevice::startAdvertising();
  }
};

int readSensor() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(10);
  }
  return sum / 10;
}

void setup() {
  Serial.begin(9600);   //  faster + standard

  BLEDevice::init("ESP32_Moisture");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // THIS fixes your "Write Not Permitted" error
  pCharacteristic->addDescriptor(new BLE2902());

  pCharacteristic->setValue("0");

  pService->start();

  // 🔧 Better advertising config
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("BLE Ready...");
}

void loop() {
  int moisture = readSensor();
  Serial.println(analogRead(SENSOR_PIN));

  // 🚨 Basic sanity check (prevents sending garbage)
  if (moisture < 50) {
    Serial.println("⚠️ Sensor not in soil / invalid reading");
   // Serial.println(analogRead(SENSOR_PIN));
    delay(2000);
    Serial.println(analogRead(SENSOR_PIN));

    return;
  }

  if (deviceConnected) {
    char valueStr[10];
    sprintf(valueStr, "%d", moisture);

    pCharacteristic->setValue(valueStr);
    pCharacteristic->notify();

    Serial.print("Sent: ");
    Serial.println(valueStr);
  }

  delay(2000);
}
