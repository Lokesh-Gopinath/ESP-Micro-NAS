#include <FS.h>
#include <LittleFS.h>

// ───────────── UART ─────────────
#define RXD1 20
#define TXD1 21

HardwareSerial espSerial(1);

// ───────────── File ─────────────
const char* NOTES_FILE = "/notes.txt";

// ───────────── Setup ─────────────
void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RXD1, TXD1);

  if (!LittleFS.begin()) {
    LittleFS.begin(true);
  }

  Serial.println("ESP32-C3 Backend Ready");
}

// ───────────── Loop ─────────────
void loop() {
  static String line = "";

  while (espSerial.available()) {
    char c = espSerial.read();

    if (c == '\n') {
      line.trim();
      Serial.println(">> " + line);
      handleCommand(line);
      line = "";
    } else {
      line += c;
    }
  }
}

// ───────────── Send Response ─────────────
void sendLine(String msg) {
  espSerial.println(msg);
  Serial.println("<< " + msg);
}

// ───────────── Command Handler ─────────────
void handleCommand(String cmd) {
  int p1 = cmd.indexOf('|');
  int p2 = cmd.indexOf('|', p1 + 1);

  String action = cmd.substring(0, p1);
  String category = (p2 > 0) ? cmd.substring(p1 + 1, p2) : cmd.substring(p1 + 1);
  String value = (p2 > 0) ? cmd.substring(p2 + 1) : "";

  if (category != "notes") {
    sendLine("ERR|Invalid category");
    sendLine("END");
    return;
  }

  if (action == "ADD") addNote(value);
  else if (action == "LIST") listNotes();
  else if (action == "SEARCH") searchNotes(value);
  else if (action == "DEL") deleteNote(value);
  else if (action == "CLEAR") clearNotes();
  else {
    sendLine("ERR|Unknown command");
    sendLine("END");
  }
}

// ───────────── ADD ─────────────
void addNote(String note) {
  File f = LittleFS.open(NOTES_FILE, "a");
  if (!f) {
    sendLine("ERR|File error");
    sendLine("END");
    return;
  }

  f.println(note);
  f.close();

  sendLine("OK|Added");
  sendLine("END");
}

// ───────────── LIST ─────────────
void listNotes() {
  File f = LittleFS.open(NOTES_FILE, "r");
  if (!f) {
    sendLine("OK|No notes");
    sendLine("END");
    return;
  }

  int i = 1;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      sendLine("DATA|" + String(i++) + "|" + line);
    }
  }

  f.close();
  sendLine("END");
}

// ───────────── SEARCH ─────────────
void searchNotes(String key) {
  File f = LittleFS.open(NOTES_FILE, "r");
  if (!f) {
    sendLine("ERR|No file");
    sendLine("END");
    return;
  }

  int i = 1;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.indexOf(key) >= 0) {
      sendLine("DATA|" + String(i++) + "|" + line);
    }
  }

  f.close();
  sendLine("END");
}

// ───────────── DELETE ─────────────
void deleteNote(String target) {
  File f = LittleFS.open(NOTES_FILE, "r");
  File temp = LittleFS.open("/temp.txt", "w");

  bool found = false;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();

    if (line == target) {
      found = true;
      continue;
    }

    if (line.length() > 0) temp.println(line);
  }

  f.close();
  temp.close();

  LittleFS.remove(NOTES_FILE);
  LittleFS.rename("/temp.txt", NOTES_FILE);

  sendLine(found ? "OK|Deleted" : "ERR|Not found");
  sendLine("END");
}

// ───────────── CLEAR ─────────────
void clearNotes() {
  LittleFS.remove(NOTES_FILE);
  sendLine("OK|Cleared");
  sendLine("END");
}
