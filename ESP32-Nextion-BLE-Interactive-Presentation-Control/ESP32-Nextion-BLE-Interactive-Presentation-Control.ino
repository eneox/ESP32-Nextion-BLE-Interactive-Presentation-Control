#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SoftwareSerial.h>
#include <Nextion.h>

SoftwareSerial tftSerial(2, 3);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

NexButton b0 = NexButton(0, 2, "b0");
NexButton b1 = NexButton(0, 3, "b1");
NexText t0 = NexText(0, 4, "t0");

NexTouch *nex_listen_list[] = {
  &b0,
  &b1,
  NULL
};

void b0PopCallback(void *ptr) {
  t0.setText("State:ON");
  digitalWrite(LED_BUILTIN, HIGH);
}

void b1PopCallback(void *ptr) {
  t0.setText("State:OFF");
  digitalWrite(LED_BUILTIN, LOW);
}

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value == "START_PRESENTATION") {
      startPresentation();
    } else if (value == "NEXT_SLIDE") {
      nextSlide();
    } else if (value == "END_PRESENTATION") {
      endPresentation();
    }
  }

  void startPresentation() {
    updateScreen("Presentation Started...");
  }

  void nextSlide() {
    updateScreen("Next Slide...");
  }

  void endPresentation() {
    updateScreen("Presentation Ended...");
  }

  void updateScreen(const char* message) {
    tftSerial.println(message);
  }
};

void setup() {
  Serial.begin(115200);
  tftSerial.begin(9600);
  nexInit();

  BLEDevice::init("Smart Board");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLEServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  pAdvertising->start();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  nexLoop(nex_listen_list);
  if (deviceConnected) {
    delay(1000);
  }
}

void updateScreen(const char* message) {
  tftSerial.println(message);
}
