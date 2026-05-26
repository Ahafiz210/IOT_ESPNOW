#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display Konfiguration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C // Falls schwarz bleibt, hier 0x3D testen
#define I2C_SDA 21
#define I2C_SCL 19

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Standby-Timer
unsigned long lastActivityTime = 0;
const unsigned long DISPLAY_TIMEOUT = 10000; // 10 Sekunden
bool displayActive = true;

WebServer server(80);

const char* DB_HOST = "10.48.110.122/"; 
const uint16_t DB_PORT = 8000;
const char* DB_PATH = "/insert";

typedef struct struct_message {
  unsigned long ms;
  int raw;
  bool bright;
} struct_message;

struct_message incomingData;
int currentRaw = 0;
bool currentBright = false;
String currentState = "-";
String lastTimestamp = "-";

volatile bool dbSendPending = false;
int pendingRaw = 0;
bool pendingBright = false;
String pendingTimestamp = "-";

void updateDisplay() {
  lastActivityTime = millis();
  displayActive = true;
  
  display.ssd1306_command(SSD1306_DISPLAYON); 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,0);
  display.println("STATION AKTIV");
  display.println(WiFi.localIP().toString());
  
  display.drawLine(0, 18, 128, 18, SSD1306_WHITE);
  
  display.setCursor(0, 25);
  display.print("Licht: ");
  display.println(currentRaw);
  display.print("Status: ");
  display.println(currentState);
  
  display.setCursor(0, 50);
  display.print("Zeit: ");
  display.print(lastTimestamp.substring(11)); 
  
  display.display();
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "-";
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void sendToDB(int raw, String timestamp, bool bright) {
  if (WiFi.status() != WL_CONNECTED) return;
  timestamp.replace(" ", "%20");
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + String(DB_HOST) + ":" + String(DB_PORT) + String(DB_PATH);
  url += "?sensor=" + String(raw) + "&timestamp=" + timestamp + "&bright=" + String(bright ? 1 : 0);
  http.begin(client, url);
  http.GET();
  http.end();
}

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingDataBytes, int len) {
  if (len != sizeof(struct_message)) return;
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  currentRaw = incomingData.raw;
  currentBright = incomingData.bright;
  currentState = currentBright ? "HELL" : "DUNKEL";
  lastTimestamp = getTimeString();

  pendingRaw = currentRaw;
  pendingBright = currentBright;
  pendingTimestamp = lastTimestamp;
  dbSendPending = true;

  updateDisplay(); 
}

void handleRoot() {
  updateDisplay(); 
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta http-equiv='refresh' content='5'>";
  html += "<title>Sensorstation</title><style>body{font-family:Arial;margin:30px;background:#f4f4f4;}.box{background:white;padding:20px;border-radius:12px;max-width:450px;box-shadow:0 0 10px rgba(0,0,0,0.1);}h1{margin-top:0;}</style></head><body>";
  html += "<div class='box'><h1>Sensorstation</h1><p><b>Lichtwert:</b> " + String(currentRaw) + "</p><p><b>Status:</b> " + currentState + "</p><p><b>Zeit:</b> " + lastTimestamp + "</p><p><b>IP:</b> " + WiFi.localIP().toString() + "</p></div></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED Fehler");
  }
  
  // Zeige sofort etwas an, damit du weisst, ob das Display geht
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Starte System...");
  display.println("WLAN Setup...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.autoConnect("ESP32-Station-Setup");

  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");

  server.on("/", handleRoot);
  server.on("/api/data", [](){
    String json = "{\"sensor\":" + String(currentRaw) + ",\"status\":\"" + currentState + "\"}";
    server.send(200, "application/json", json);
  });
  server.begin();

  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(onDataRecv);
  }
  
  updateDisplay();
}

void loop() {
  server.handleClient();
  if (dbSendPending) {
    dbSendPending = false;
    sendToDB(pendingRaw, pendingTimestamp, pendingBright);
  }
  if (displayActive && (millis() - lastActivityTime > DISPLAY_TIMEOUT)) {
    display.ssd1306_command(SSD1306_DISPLAYOFF); 
    displayActive = false;
  }
}
