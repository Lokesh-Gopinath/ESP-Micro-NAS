# Architecture Overview

## System Design

This project follows a distributed architecture:

[ Termius ]
↓
[ ESP8266 ]  →  [ ESP32-C3 ]

## Components

### ESP8266 (Frontend)

* Telnet server
* CLI interface
* Command parsing
* UART communication

### ESP32-C3 (Backend)

* Command execution
* File storage (LittleFS)
* Data processing

## Communication Protocol

Format:

COMMAND|category|value

Example:

ADD|notes|Buy milk

Response:

OK|Added
DATA|1|Buy milk
END

## Design Goals

* Low memory usage
* Simple text-based protocol
* Scalable architecture
* Separation of concerns

## Limitations (v0.0.1)

* Local network only
* Single category (notes)
* No encryption
