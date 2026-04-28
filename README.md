# ESP32 Temperature Control System

## Overview
This project implements a touchscreen-based temperature control system using ESP32.  
It monitors environmental conditions and controls a valve system based on temperature and dew point logic.

## Features
- Real-time temperature and humidity monitoring
- Touchscreen GUI interface
- Automatic valve control logic
- Sensor integration (DHT22, PT100)
- Communication with external devices (ESP32 nodes)

## Hardware Used
- ESP32
- TFT Display (SPI / Parallel)
- XPT2046 Touch Controller
- DHT22 Sensor
- PT100 (via MAX31865)
- Relay / Valve control system

## Software & Tools
- Arduino IDE (ESP32)
- Embedded C/C++
- TFT_eSPI / LVGL (GUI)
- SPI Communication

## System Design
The system reads:
- Room Temperature
- Humidity
- Dew Point
- Floor Temperature

Based on these values, control logic determines whether the valve should be ON or OFF:
- If set temperature achieved → Valve OFF  
- If floor temp ≤ dew point + offset → Valve OFF  
- Otherwise → Valve ON  

## How It Works
1. Sensors collect environmental data  
2. ESP32 processes values  
3. GUI displays readings on touchscreen  
4. Control logic decides valve operation  
5. Output signal sent to relay/ESP32 node  

## Results
(Add screenshots of your GUI + system running)

## Folder Structure
src/ - main ESP32 code  
lib/ - libraries (if any)  
docs/ - design explanation  
images/ - screenshots  

## Author
Calvin D'Cruz
