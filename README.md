# ESP32 Touchscreen Temperature Control System

## Overview
This project implements a smart temperature control system using ESP32 with a touchscreen interface.  
It monitors environmental conditions and controls a relay (valve system) based on temperature and dew point logic.

## Features
- Real-time temperature and humidity monitoring (DHT22)
- Floor temperature sensing using PT100 (MAX31865)
- Touchscreen GUI with multiple screens
- Adjustable control parameters using sliders
- Automatic relay control based on environmental conditions

## System Design

### Sensors:
- Room Temperature & Humidity → DHT22  
- Floor Temperature → PT100 via MAX31865  

### Control Logic:
- If floor temperature < (dew point + offset) → Relay OFF  
- If room temperature > cut-in temperature → Relay ON  
- If room temperature < cut-out temperature → Relay OFF  

### User Interface:
- Main Screen:
  - Displays Room Temp, Humidity, Dew Point, Floor Temp
- Settings Screen:
  - Adjust Cut-In Temp
  - Adjust Cut-Out Temp
  - Adjust Dew Offset

## Tools & Technologies
- ESP32 (Arduino Framework)
- TFT_eSPI (Display)
- XPT2046 (Touch Controller)
- Adafruit MAX31865 Library
- Embedded C++

## How to Run
1. Open code in Arduino IDE  
2. Install required libraries:
   - TFT_eSPI
   - XPT2046_Touchscreen
   - DHT
   - Adafruit MAX31865  
3. Upload to ESP32  
4. Interact using touchscreen  

## Results

### Main Screen
![Main Screen](images/main_screen.png)

### Settings Screen
![Settings](images/settings_screen.png)

## Folder Structure
src/ - main ESP32 code  
images/ - screenshots  
docs/ - design notes  

## Author
Calvin D'Cruz
