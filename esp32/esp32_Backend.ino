#include <FS.h>
#include <LittleFS.h>

#define RXD1 20
#define TXD1 21

HardwareSerial espSerial(1);

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RXD1, TXD1);

  if (!LittleFS.begin()) LittleFS.begin(true);

  Serial.println("ESP32 Backend Ready");
}

// ───────────── Loop ─────────────
void loop() {
  static String line = "";

  while (espSerial.available()) {
    char c = espSerial.read();

    if (c == '\n') {
      line.trim();
      handleCommand(line);
      line = "";
    } else {
      line += c;
    }
  }
}

// ───────────── Helpers ─────────────
void sendLine(String msg) {
  espSerial.println(msg);
}

String path(String cat) {
  return "/" + cat + ".txt";
}

// ───────────── Command Handler ─────────────
void handleCommand(String cmd) {

  //  Special command: categories list
  if (cmd == "CATEGORIES") {
    listCategories();
    return;
  }

  int p1 = cmd.indexOf('|');
  int p2 = cmd.indexOf('|', p1 + 1);
  int p3 = cmd.indexOf('|', p2 + 1);

  String action = cmd.substring(0, p1);
  String cat = (p2 < 0) ? cmd.substring(p1 + 1) : cmd.substring(p1 + 1, p2);

  String v1 = "", v2 = "";

  if (p2 >= 0) {
    if (p3 < 0) v1 = cmd.substring(p2 + 1);
    else {
      v1 = cmd.substring(p2 + 1, p3);
      v2 = cmd.substring(p3 + 1);
    }
  }

  action.trim(); cat.trim(); v1.trim(); v2.trim();

  if (action == "ADD") addItem(cat, v1);
  else if (action == "LIST") listItems(cat);
  else if (action == "SEARCH") searchItems(cat, v1);
  else if (action == "DEL") deleteItem(cat, v1);
  else if (action == "CLEAR") clearItems(cat);
  else if (action == "EDIT") editItem(cat, v1, v2);
  else {
    sendLine("ERR|Unknown");
    sendLine("END");
  }
}

// ───────────── CATEGORY LIST ─────────────
void listCategories() {
  File root = LittleFS.open("/");
  File file = root.openNextFile();

  int i = 1;

  while (file) {
    String name = file.name();  // e.g. "/anime.txt"

    if (name.endsWith(".txt")) {
      name.replace("/", "");
      name.replace(".txt", "");
      sendLine("DATA|" + String(i++) + "|" + name);
    }

    file = root.openNextFile();
  }

  sendLine("END");
}

// ───────────── CRUD ─────────────
void addItem(String cat, String val) {
  File f = LittleFS.open(path(cat), "a");
  f.println(val);
  f.close();
  sendLine("OK|Added");
  sendLine("END");
}

void listItems(String cat) {
  File f = LittleFS.open(path(cat), "r");
  int i = 1;

  while (f.available()) {
    String l = f.readStringUntil('\n'); l.trim();
    if (l.length()) sendLine("DATA|" + String(i++) + "|" + l);
  }

  f.close();
  sendLine("END");
}

void searchItems(String cat, String key) {
  File f = LittleFS.open(path(cat), "r");
  int i = 1;

  while (f.available()) {
    String l = f.readStringUntil('\n'); l.trim();
    if (l.indexOf(key) >= 0)
      sendLine("DATA|" + String(i++) + "|" + l);
  }

  f.close();
  sendLine("END");
}

void deleteItem(String cat, String target) {
  File f = LittleFS.open(path(cat), "r");
  File t = LittleFS.open("/tmp.txt", "w");

  bool found = false;

  while (f.available()) {
    String l = f.readStringUntil('\n'); l.trim();

    if (l == target) { found = true; continue; }
    if (l.length()) t.println(l);
  }

  f.close(); t.close();

  LittleFS.remove(path(cat));
  LittleFS.rename("/tmp.txt", path(cat));

  sendLine(found ? "OK|Deleted" : "ERR|Not found");
  sendLine("END");
}

void clearItems(String cat) {
  LittleFS.remove(path(cat));
  sendLine("OK|Cleared");
  sendLine("END");
}

void editItem(String cat, String oldVal, String newVal) {
  File f = LittleFS.open(path(cat), "r");
  File t = LittleFS.open("/tmp.txt", "w");

  bool found = false;

  oldVal.trim();
  newVal.trim();

  while (f.available()) {
    String l = f.readStringUntil('\n');
    l.trim();

    String cleanLine = l;

    if (cleanLine.equalsIgnoreCase(oldVal)) {
      t.println(newVal);
      found = true;
    } else if (l.length() > 0) {
      t.println(l);
    }
  }

  f.close(); t.close();

  LittleFS.remove(path(cat));
  LittleFS.rename("/tmp.txt", path(cat));

  sendLine(found ? "OK|Edited" : "ERR|Not found");
  sendLine("END");
}
