#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const int RELAY_PIN = 2;
const char* WIFI_NAME = "MI_PI";
const char* WIFI_PASSWORD = "3.1415926+";
const int MONITOR_SPEED = 9600;
const int SERVER_PORT = 80;

IPAddress staticIP(192, 168, 31, 7); 
IPAddress gateway(192, 168, 31, 1);    
IPAddress subnet(255, 255, 255, 0);   

ESP8266WebServer server(SERVER_PORT);

void press_button(int time) {
  digitalWrite(RELAY_PIN, LOW);
  delay(time);
  digitalWrite(RELAY_PIN, HIGH);
}

void press_power_button() {
  press_button(1000);
  Serial.println("Press power button.");
}

void press_reset_button() {
  press_button(3000);
  Serial.println("Press reset button.");
}
void router_home() {
  String html = "<html><head>";
  html += "<style>body { display: flex; justify-content: center; align-items: center; height: 100vh; }";
  html += "button { margin: 10px; padding: 10px 20px; font-size: 16px; border-radius: 5px; background-color: #007bff; color: #fff; border: none; cursor: pointer; }";
  html += "button:hover { background-color: #0056b3; }";
  html += "button:disabled { cursor: not-allowed; }";
  html += "#resetBtn { background-color: #dc3545; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div style='text-align: center;'>";
  html += "<h1>ESP8266 WATCHDOG</h1>";
  html += "<div style='display: flex; justify-content: center;'>";
  html += "<button id='powerBtn' onclick=\"press('/power', 'powerBtn')\">Power</button>";
  html += "<button id='resetBtn' onclick=\"press('/reset', 'resetBtn')\">Reset</button>";
  html += "</div>";
  html += "<script>";
  html += "function press(url, btnId) {";
  html += "var btn = document.getElementById(btnId);";
  html += "btn.disabled = true;";
  html += "btn.innerHTML = 'Loading...';";
  html += "fetch(url, {method: 'POST'}).then(() => {";
  html += "btn.innerHTML = btnId === 'powerBtn' ? 'Power' : 'Reset';";
  html += "btn.disabled = false;";
  html += "}).catch(err => {";
  html += "btn.innerHTML = 'Error';";
  html += "btn.disabled = false;";
  html += "alert(err.toString());";
  html += "});";
  html += "}";
  html += "</script>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void router_power() {
  press_power_button();
  server.send(200, "text/html", "OK");
}

void router_reset() {
  press_reset_button();
  server.send(200, "text/html", "OK");
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.begin(MONITOR_SPEED);

  Serial.print("Connecting to the network...");
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("connection succeeded.");
  Serial.print("IP: ");

  Serial.println(WiFi.localIP());

  // After powering on for 3 seconds, press the power button
  delay(3000);
  press_power_button();

  // Create web server
  server.on("/", router_home);
  server.on("/power", HTTP_POST, router_power);
  server.on("/reset", HTTP_POST, router_reset);

  server.begin();

  Serial.println("Web server is started.");
}

void loop() {
  server.handleClient();
}
