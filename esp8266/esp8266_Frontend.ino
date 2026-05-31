#include <ESP8266WiFi.h>
#include <ESPTelnet.h>
#include <SoftwareSerial.h>

// ───────────── WiFi ─────────────
const char* ssid = "Your WiFi ssid";
const char* password = "Your WiFi password";

// ───────────── Telnet ─────────────
ESPTelnet telnet;
bool authenticated = false;

const char* user = "admin";
const char* pass = "1234";

// ───────────── Static IP ─────────────
IPAddress local_IP(192, 168, 0, 105);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// ───────────── UART ─────────────
SoftwareSerial espSerial(D5, D6);

// ───────────── Setup ─────────────
void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

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
        telnet.println("\n[OK] Logged in");
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
  cmd.trim();

  // LOGOUT COMMAND
  if (cmd == "logout") {
      authenticated = false;
      telnet.println("\nLogged out.");
      telnet.print("User: ");
      return;
  }

  // HELP COMMAND (local, handled on ESP8266)
  if (cmd == "help") {
    telnet.println("\nAvailable Commands:\n");

    telnet.println("<category> add <data>");
    telnet.println("<category> del <data>");
    telnet.println("<category> edit <old> <new>");
    telnet.println("<category> list");
    telnet.println("<category> search <data>");
    telnet.println("<category> clear");
    telnet.println("categories list");

    telnet.println("\nExamples:");
    telnet.println("notes add test");
    telnet.println("notes list");
    telnet.println("notes edit test text");

    telnet.print("> ");
    return;
  }

  // CATEGORY LIST
  if (cmd == "categories list") {
    espSerial.println("CATEGORIES");
  return;
  }

  // NORMAL COMMAND FLOW
  String protocol = buildProtocol(cmd);

  if (protocol.startsWith("ERR")) {
    telnet.println("[ERR] Invalid command");
    telnet.print("> ");
    return;
  }

  Serial.println(">> " + protocol);
  espSerial.println(protocol);
}

// ───────────── CLI → Protocol ─────────────
String buildProtocol(String input) {
  input.trim();

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

  if (action == "edit") {
    int split = value.indexOf(' ');
    if (split < 0) return "ERR|Invalid edit";

    String oldVal = value.substring(0, split);
    String newVal = value.substring(split + 1);

    return "EDIT|" + category + "|" + oldVal + "|" + newVal;
  }

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
    telnet.println("[OK] " + line.substring(3));
  }
  else if (line.startsWith("ERR|")) {
    telnet.println("[ERR] " + line.substring(4));
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
