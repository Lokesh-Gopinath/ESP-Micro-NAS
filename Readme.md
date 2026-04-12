# ESP Micro NAS (v0.0.1)

A distributed embedded system built using ESP8266 and ESP32-C3 that provides a Telnet-based command interface with persistent storage.

## 🚀 Features

* Telnet CLI interface (ESP8266)
* Custom command protocol (UART communication)
* Backend processing and storage (ESP32-C3)
* Persistent notes system (LittleFS)
* Multi-word command support

## 🏗️ Architecture

ESP8266 acts as the frontend (network + CLI)
ESP32-C3 acts as the backend (processing + storage)

Communication happens over UART using a custom protocol.

## 🔌 Wiring

ESP8266 D6 (TX) → ESP32 GPIO20 (RX)
ESP8266 D5 (RX) ← ESP32 GPIO21 (TX)
GND ↔ GND

## 📟 Commands

notes add <text>
notes list
notes search <text>
notes del <text>
notes clear

## 🔧 Tech Stack

* ESP8266 (WiFi + Telnet)
* ESP32-C3 (Processing + Storage)
* LittleFS (File system)
* Custom UART protocol

## 📌 Version

v0.0.1 — Initial working system (local access only)

## 🧠 Future Plans

* Multi-category support (anime, movies)
* Timestamped notes
* SD card storage
* Remote access via custom tunnel system

## 📄 License

MIT
