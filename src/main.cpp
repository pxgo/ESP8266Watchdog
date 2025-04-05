#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

WiFiUDP udp;

// 广播地址和端口号
const char* broadcastIP = "255.255.255.255";
const int port = 9;

const int RELAY_PIN = 2;
const char* WIFI_NAME = "********";
const char* WIFI_PASSWORD = "*******";
const int MONITOR_SPEED = 9600;
const int SERVER_PORT = 80;

IPAddress staticIP(192, 168, 31, 7); 
IPAddress gateway(192, 168, 31, 1);    
IPAddress subnet(255, 255, 255, 0);   

ESP8266WebServer server(SERVER_PORT);

void sendWakeOnLan(const String& mac) {
  uint8_t macBytes[6];
  
  // 将 MAC 地址从字符串转换为字节数组
  sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
         &macBytes[0], &macBytes[1], &macBytes[2], 
         &macBytes[3], &macBytes[4], &macBytes[5]);

  // 构建魔术包
  uint8_t magicPacket[102];
  memset(magicPacket, 0xFF, 6);
  for (int i = 1; i <= 16; i++) {
    memcpy(&magicPacket[i * 6], macBytes, 6);
  }

  // 发送魔术包
  udp.beginPacket(broadcastIP, port);
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();

  Serial.println("Wake-on-LAN 魔术包已发送");
}

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

void router_wake() {
  // 检查是否提供了 MAC 参数
  if (!server.hasArg("mac")) {
    server.send(400, "text/plain", "Missing 'mac' parameter");
    return;
  }

  // 获取 MAC 地址
  String mac = server.arg("mac");

  // 验证 MAC 地址格式
  if (mac.length() != 17 || mac[2] != ':' || mac[5] != ':' || 
      mac[8] != ':' || mac[11] != ':' || mac[14] != ':') {
    server.send(400, "text/plain", "Invalid MAC address format");
    return;
  }

  // 调用发送魔术包函数
  sendWakeOnLan(mac);

  // 返回成功响应
  server.send(200, "text/plain", "Magic packet sent to " + mac);
}


void router_msi() {
  // 固定的 MAC 地址
  String mac = "00:D8:61:13:46:C7";

  // 发送魔术包
  sendWakeOnLan(mac);

  // 返回响应
  server.send(200, "text/plain", "Magic packet sent to 00:D8:61:13:46:C7");
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
  server.on("/wake", HTTP_GET, router_wake);
  server.on("/msi", HTTP_GET, router_msi);

  server.begin();

  Serial.println("Web server is started.");
}

void loop() {
  server.handleClient();
}
