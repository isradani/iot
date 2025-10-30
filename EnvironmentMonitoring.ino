#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const char* ssid = "amar";
const char* password = "amar0812";
WebServer server(80);
const int mq135Pin = 34;
const int ldrPin = 35;
const int ledPin = 26;
const int buzzerPin = 27;
const int LDR_THRESHOLD = 1500;
const int MQ_THRESHOLD = 1800;
void setup() {
Serial.begin(115200);
dht.begin();
pinMode(ledPin, OUTPUT);
pinMode(buzzerPin, OUTPUT);
digitalWrite(ledPin, LOW);
digitalWrite(buzzerPin, LOW);
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
}
server.on("/", handleRoot);
server.begin();
}
void loop() {
server.handleClient();
int ldrVal = analogRead(ldrPin);
int mq135 = analogRead(mq135Pin);
if (ldrVal < LDR_THRESHOLD) digitalWrite(ledPin, HIGH);
else digitalWrite(ledPin, LOW);
if (mq135 > MQ_THRESHOLD) digitalWrite(buzzerPin, HIGH);
else digitalWrite(buzzerPin, LOW);
delay(100);
}
void handleRoot() {
float temp = dht.readTemperature();
float hum = dht.readHumidity();
int mq135 = analogRead(mq135Pin);
int ldrVal = analogRead(ldrPin);
String html = "<html><body><h2>Environment Monitoring</h2>";
html += "<p>Temperature: " + String(temp) + " °C</p>";
html += "<p>Humidity: " + String(hum) + " %</p>";
html += "<p>Air Quality (MQ-135): " + String(mq135) + "</p>";
html += "<p>Light Level (LDR): " + String(ldrVal) + "</p>";
html += "</body></html>";
server.send(200, "text/html", html);
}
