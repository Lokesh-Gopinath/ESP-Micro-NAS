#include <ESP8266WiFi.h>
#include <ESPTelnet.h>
#include <SoftwareSerial.h>

// ───────────── WiFi ─────────────
const char* ssid = "Your WiFi ssid/name";
const char* password = "Your WiFi Password";

// ───────────── Telnet ─────────────
ESPTelnet telnet;
bool authenticated = false;

const char* user = "admin";
const char* pass = "1234";

// ───────────── Static IP Config ─────────────
IPAddress local_IP(192, 168, 0, 105); // replace the ip address that is free(not assigned to anyother devices) on your router
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// ───────────── UART (to ESP32) ─────────────
// RX, TX
SoftwareSerial espSerial(D5, D6);

// ───────────── Setup ─────────────
void setup() {
  Serial.begin(115200);     // debug
  espSerial.begin(115200);  // UART to ESP32

  // Apply static IP
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Static IP Failed");
  }

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println("IP: " + WiFi.localIP().toString());

  telnet.onConnect([](String ip) {
    telnet.println("=== ESP8266 Terminal ===");
    telnet.print("User: ");
    authenticated = false;
  });

  telnet.onDisconnect([](String ip) {
    authenticated = false;
  });

  telnet.onInputReceived(onTelnetInput);
  telnet.begin(2323);

  Serial.println("Telnet ready");
}

// ───────────── Loop ─────────────
void loop() {
  telnet.loop();
  readFromESP32();
}

// ───────────── Telnet Input ─────────────
void onTelnetInput(String input) {
  input.trim();

  if (!authenticated) {
    static bool waitPass = false;

    if (!waitPass) {
      if (input == user) {
        waitPass = true;
        telnet.print("Password: ");
      } else {
        telnet.println("Wrong user");
        telnet.print("User: ");
      }
    } else {
      if (input == pass) {
        authenticated = true;
        waitPass = false;
        telnet.println("\n✓ Logged in");
        telnet.print("> ");
      } else {
        telnet.println("Wrong password");
        telnet.print("User: ");
        waitPass = false;
      }
    }
    return;
  }

  handleCommand(input);
}

// ───────────── Command Handler ─────────────
void handleCommand(String cmd) {
  String protocol = buildProtocol(cmd);

  if (protocol.startsWith("ERR")) {
    telnet.println("✗ Invalid command");
    telnet.print("> ");
    return;
  }

  Serial.println(">> " + protocol);
  espSerial.println(protocol);
}

// ───────────── CLI → Protocol ─────────────
String buildProtocol(String input) {
  input.trim();
  input.toLowerCase();

  int firstSpace = input.indexOf(' ');
  if (firstSpace < 0) return "ERR|Invalid";

  String category = input.substring(0, firstSpace);
  String rest = input.substring(firstSpace + 1);

  int secondSpace = rest.indexOf(' ');
  String action = (secondSpace < 0) ? rest : rest.substring(0, secondSpace);
  String value  = (secondSpace < 0) ? "" : rest.substring(secondSpace + 1);

  if (action == "add" && value.length() > 0)
    return "ADD|" + category + "|" + value;

  if (action == "list")
    return "LIST|" + category;

  if (action == "del" && value.length() > 0)
    return "DEL|" + category + "|" + value;

  if (action == "search" && value.length() > 0)
    return "SEARCH|" + category + "|" + value;

  if (action == "clear")
    return "CLEAR|" + category;

  return "ERR|Unknown";
}

// ───────────── Read Response ─────────────
void readFromESP32() {
  static String line = "";

  while (espSerial.available()) {
    char c = espSerial.read();

    if (c == '\n') {
      line.trim();
      handleResponse(line);
      line = "";
    } else {
      line += c;
    }
  }
}

// ───────────── Handle Response ─────────────
void handleResponse(String line) {
  Serial.println("<< " + line);

  if (line.startsWith("OK|")) {
    telnet.println("✓ " + line.substring(3));
  }
  else if (line.startsWith("ERR|")) {
    telnet.println("✗ " + line.substring(4));
  }
  else if (line.startsWith("DATA|")) {
    int p = line.indexOf('|', 5);
    if (p > 0) {
      telnet.println(line.substring(5, p) + ") " + line.substring(p + 1));
    }
  }
  else if (line == "END") {
    telnet.print("> ");
  }
}
