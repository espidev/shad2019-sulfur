#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define PIN 12

const char* ssid = "poo";
const char* password = "3333333333";

float calibrationFactor = 4.5;
volatile byte pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;

void setup() {

  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);

  pinMode(0, OUTPUT); // LED
  digitalWrite(0, HIGH);

  delay(1000);
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  attachInterrupt(digitalPinToInterrupt(PIN), pulseCounter, FALLING);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://stream.espi.dev/update");
    int httpCode = http.POST(String("{\"flow_volume\": ") + int(flowRate) + ", \"total_volume\": " + totalMilliLitres + "}");
    if (httpCode > 0) {
      Serial.println("POST good! code: " + httpCode);
    } else {
      Serial.println("POST failed! code: " + httpCode);
    }
    http.end();
  }


  if ((millis() - oldTime) > 1000) { // once per second
    detachInterrupt(0);

    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    unsigned int frac;

    Serial.print("\nFlow rate: ");
    Serial.print(int(flowRate));
    Serial.print("L/min");
    Serial.print("\t");

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.println("mL");
    Serial.print("\t");
    Serial.print(totalMilliLitres / 1000);
    Serial.print("L");

    pulseCount = 0;

    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(PIN), pulseCounter, FALLING);
  }
  delay(50);
}

void pulseCounter() {
  pulseCount++;
}