#include <WiFi.h>
#include <esp_now.h>

// MAC-Adresse vom EMPFAENGER-ESP32 eintragen
uint8_t receiverMac[] = {0x68, 0xFE, 0x71, 0x87, 0xE5, 0x30};

// RGB LED
const int redPin = 32;
const int greenPin = 23;
const int bluePin = 33;

// Lichtsensor
const int lightPin = 35;

// Schwellwert
const int THRESHOLD = 2000;

typedef struct struct_message {
  unsigned long ms;
  int raw;
  bool bright;
} struct_message;

struct_message data;

void setColor(bool r, bool g, bool b) {
  digitalWrite(redPin, r);
  digitalWrite(greenPin, g);
  digitalWrite(bluePin, b);
}

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Sendestatus: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FEHLER");
}

void setup() {
  Serial.begin(115200);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(lightPin, INPUT);

  WiFi.mode(WIFI_STA);

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Fehler");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;   
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Peer konnte nicht hinzugefuegt werden");
    return;
  }

  Serial.println("Sender bereit");
}

void loop() {
  long sum = 0;
  const int samples = 10;
  
  for (int i = 0; i < samples; i++) {
    sum += analogRead(lightPin);
    delay(10);
  }

  int lightRaw = sum / samples;
  bool isBright = lightRaw < THRESHOLD;

  data.ms = millis();
  data.raw = lightRaw;
  data.bright = isBright;

  Serial.print("RAW: ");
  Serial.print(lightRaw);
  Serial.print(" | Status: ");
  Serial.println(isBright ? "HELL" : "DUNKEL");

  if (isBright) {
    setColor(0, 1, 0);   // gruen
  } else {
    setColor(1, 0, 0);   // rot
  }

  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&data, sizeof(data));
  
  if (result != ESP_OK) {
    Serial.print("esp_now_send Fehler: ");
    Serial.println(result);
  }

  delay(5000); // 5 Sekunden warten
}
