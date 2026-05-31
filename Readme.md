# ESP Micro NAS (v0.0.3)

A distributed embedded system built using **ESP8266** and **ESP32-C3** that provides a **Telnet-based command interface** with **persistent storage** and **multi-category support**.

---

## 🚀 Features

- **Telnet CLI interface** (ESP8266) with **authentication** (username: `admin`, password: `1234`)
- **Custom command protocol** (UART communication)
- **Backend processing and storage** (ESP32-C3)
- **Persistent notes system** (LittleFS)
- **Multi-category support** (e.g., `notes`, `anime`, `movies`)
- **Multi-word command support**
- **Case-insensitive editing** for items
- **Help and logout commands** for better user experience

---

## 🏗️ Architecture

- **ESP8266** acts as the frontend (network + CLI)
- **ESP32-C3** acts as the backend (processing + storage)
- **Communication** happens over **UART** using a custom protocol.

---

## 🔌 Wiring

ESP8266 D6 (TX) → ESP32 GPIO20 (RX)
ESP8266 D5 (RX) ← ESP32 GPIO21 (TX)
GND ↔ GND

---

## 📟 Commands

### **Category Management**
- `categories list` — List all available categories.

### **Item Management**
- `<category> add <text>` — Add an item to a category.
- `<category> list` — List all items in a category.
- `<category> search <text>` — Search for items containing `<text>`.
- `<category> del <text>` — Delete an item from a category.
- `<category> clear` — Clear all items in a category.
- `<category> edit <old> <new>` — Edit an item (case-insensitive matching).

### **Session Management**
- `help` — Display available commands and examples.
- `logout` — Log out of the current session.

---

## 🔧 Tech Stack

- **ESP8266** (WiFi + Telnet)
- **ESP32-C3** (Processing + Storage)
- **LittleFS** (File system)
- **Custom UART protocol**

---

## 📌 Version

**v0.0.3** — Multi-category support, case-insensitive editing, and improved CLI experience.

---

## 🧠 Future Plans

- Timestamped notes
- SD card storage
- Remote access via custom tunnel system
- Web-based interface

---
## 📄 License

MIT
