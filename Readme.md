# 🌱 Smart Agriculture System using Embedded Control and IoT

This project is a complete real-time Smart Irrigation System designed using **ESP32**, **sensor feedback**, **MQTT-based communication**, and a live **web dashboard**, aimed at sustainable agriculture through automation.

---

## 📁 Project Structure

SmartAgriProject/
│
├── simulation/ # Phase 1: Python simulation of sensor/environment data
│ └── simulate_system.py
│
├── esp32_code/ # Phase 2-3: Embedded firmware for ESP32
│ └── smart_agri.ino
│
├── dashboard/ # Phase 4: HTML + JS dashboard
│ └── index.html
│
├── logger/ # Phase 5: Python MQTT data logger
│ ├── mqtt_logger.py
│ └── *.csv
│
├── docs/ # Phase 6: Reports, diagrams, and mapping
│ ├── Design Report Smart Agri.pdf
│ └── CircuitDiagram.png
│
└── README.md # This file


---

## 🎯 Objective

Design and implement an embedded smart irrigation system that:
- Monitors environmental parameters (temperature, humidity, soil moisture)
- Predicts evaporation loss
- Irrigates based on control logic
- Communicates with a cloud dashboard
- Logs key events locally

---

## 🔄 Project Phases & Features

### ✅ Phase 1: Simulation (Python)
- Simulates temperature, humidity, and soil moisture over time
- Implements empirical evaporation logic
- Visualizes irrigation needs

### ✅ Phase 2: ESP32 Control Logic
- Reads data from:
  - DHT11 sensor (temperature & humidity)
  - Soil moisture sensor (analog)
- Computes evaporation loss
- Makes irrigation decisions
- LED acts as irrigation actuator

### ✅ Phase 3: MQTT Communication
- Uses `PubSubClient` to publish JSON payloads every 2s to:
  - `esp32/agri/data` (sensor & control state)
- Subscribes to:
  - `esp32/agri/command` for control override

### ✅ Phase 4: Web Dashboard
- Built with **HTML**, **Chart.js**, and **MQTT.js**
- Displays:
  - Live temperature, humidity, moisture, evaporation, water poured
  - Irrigation state and manual mode
- Includes **Resume button** to control ESP remotely

### ✅ Phase 5: Logging System (Python)
- Subscribes to MQTT data
- Logs:
  - `mqtt_log.csv`: All incoming data
  - `irrigation_log.csv`: When irrigation starts
  - `manual_mode_log.csv`: When manual mode is entered or exited

### ✅ Phase 6: Embedded Feasibility
- Sensor pin mappings and GPIO configuration
- Designed for low-power embedded environments
- Mapped real-world control system to microcontroller-friendly logic

---

## ⚙️ Technologies Used

- ESP32 (Arduino IDE)
- DHT11 Sensor, Soil Moisture Sensor
- MQTT (broker.emqx.io)
- HTML/CSS/JavaScript
- Python 3 (`paho-mqtt`, `csv`)
- Chart.js for visualization
- MQTT.js for web MQTT

---

## 📡 MQTT Topics Used

| Topic                  | Direction | Purpose                  |
|------------------------|-----------|--------------------------|
| `esp32/agri/data`      | Publish   | Sends sensor/control data |
| `esp32/agri/command`   | Subscribe | Receives resume command   |

---

## 📊 Sample JSON Payload

```json
{
  "temperature": 26.4,
  "humidity": 55.2,
  "soil_moisture": 38,
  "evaporation_ml": 12.3,
  "water_poured": 4.2,
  "irrigation": true,
  "manual_mode": false
}

📌 How to Run
1. ESP32 Setup
Flash smart_agri.ino via Arduino IDE

Connect DHT11 to digital pin (e.g. D4), Soil Sensor to analog pin (e.g. A0 or 34)

2. Web Dashboard
Open dashboard/index.html in browser

Ensure internet access (uses broker.emqx.io)

3. Python Logger
```bash
cd logger
pip install paho-mqtt==1.6.1
python mqtt_logger.py

