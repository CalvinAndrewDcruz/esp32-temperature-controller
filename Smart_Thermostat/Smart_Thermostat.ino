// ESP32 Temperature Controller with Touch + Display

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>

// -------------------- Pins --------------------
#define DHTPIN 14
#define DHTTYPE DHT22
#define TOUCH_CS 5
#define MAX31865_CS 17
#define RELAY_PIN 25

// -------------------- Display SPI Pins (set in User_Setup.h) --------------------
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4

// Slider bounds
#define SLIDER_X 20
#define SLIDER_W 280
#define SLIDER_H 20

#define CUT_TEMP_Y 90
#define CUT_OUT_Y 130
#define DEW_OFF_Y 170

// -------------------- Objects --------------------
DHT dht(DHTPIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS);
Adafruit_MAX31865 thermo = Adafruit_MAX31865(MAX31865_CS);

// -------------------- State --------------------
float cutInTemp = 28.0, cutOutTemp = 25.0, dewOffset = 0.0;
bool settingsUnlocked = false;
float roomTemp = 0.0, humidity = 0.0, dewPoint = 0.0, floorTemp = 0.0;
bool relayState = false;
int currentScreen = 1; // 1 = Main, 3 = Settings

// -------------------- UI Constants --------------------
#define BUTTON_W 100
#define BUTTON_H 35
#define BUTTON_X 120
#define BUTTON_Y (240 - BUTTON_H - 5)

// -------------------- Functions --------------------
float calculateDewPoint(float t, float h) {
  return t - ((100 - h) / 5.0);
}

void updateSensors() {
  roomTemp = dht.readTemperature();
  humidity = dht.readHumidity();
  dewPoint = calculateDewPoint(roomTemp, humidity);
  floorTemp = thermo.temperature(100, 430); // PT100

  Serial.println("---- Sensor Data ----");
  Serial.printf("Room Temp: %.1f C\n", roomTemp);
  Serial.printf("Humidity: %.1f %%\n", humidity);
  Serial.printf("Dew Point: %.1f C\n", dewPoint + dewOffset);
  Serial.printf("Floor Temp: %.1f C\n", floorTemp);

  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault byte: "); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) Serial.println("RTD High Threshold");
    if (fault & MAX31865_FAULT_LOWTHRESH) Serial.println("RTD Low Threshold");
    if (fault & MAX31865_FAULT_REFINLOW)  Serial.println("REFIN- < 0.85 x Bias");
    if (fault & MAX31865_FAULT_REFINHIGH) Serial.println("REFIN- > 0.85 x Bias – Possibly open");
    if (fault & MAX31865_FAULT_RTDINLOW)  Serial.println("RTDIN- < 0.85 x Bias – Open RTD?");
    if (fault & MAX31865_FAULT_OVUV)      Serial.println("Under/Over Voltage fault");
    thermo.clearFault();
  }
}

void updateRelayLogic() {
  if (floorTemp < (dewPoint + dewOffset)) relayState = false;
  else if (roomTemp > cutInTemp) relayState = true;
  else if (roomTemp < cutOutTemp) relayState = false;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.println(relayState ? "Relay: ON" : "Relay: OFF");
}

void drawDividers() {
  tft.drawLine(tft.width()/2, 0, tft.width()/2, tft.height(), TFT_BLACK);
  tft.drawLine(0, tft.height()/2, tft.width(), tft.height()/2, TFT_BLACK);
}

void drawButton(const char* label) {
  tft.fillRect(BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, TFT_LIGHTGREY);
  tft.drawRect(BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, TFT_BLACK); 
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, BUTTON_X + BUTTON_W/2, BUTTON_Y + BUTTON_H/2);
}

void drawMainScreen() {
  tft.fillScreen(TFT_CYAN);
  drawDividers();

  int w = tft.width() / 2;
  int h = tft.height() / 2;

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_BLACK);
  //tft.setTextSize(2);

  // Top Left: Room Temp
  tft.setTextSize(2.7);
  tft.drawString("Room Temp", w / 2, 10);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4.8);
  tft.drawString(String(roomTemp, 1) + "C", w / 2, h / 2);

  // Top Right: Humidity
  tft.setTextSize(2.7);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Humidity", w + w / 2, 10);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4.8);
  tft.drawString(String(humidity, 1) + "%", w + w / 2, h / 2);
   
  // Bottom Left: Dew Point
  tft.setTextSize(2.7);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Dew Point", w / 2, h + 10);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4.8);
  tft.drawString(String(dewPoint + dewOffset, 1) + "C", w / 2, h + h / 2);

  // Bottom Right: Floor Temp
  tft.setTextSize(2.7);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Floor Temp", w + w / 2, h + 10);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4.8);
  tft.drawString(String(floorTemp, 1) + "C", w + w / 2, h + h / 2);

  drawButton("Settings");
}
void drawSlider(float value, float minVal, float maxVal, int y, const char* label) {
  int xPos = SLIDER_X + ((value - minVal) / (maxVal - minVal)) * SLIDER_W;

  // Background
  tft.fillRect(SLIDER_X, y, SLIDER_W, SLIDER_H, TFT_WHITE);
  // Slider value bar
  tft.fillRect(SLIDER_X, y, xPos - SLIDER_X, SLIDER_H, TFT_BLUE);
  // Border
  tft.drawRect(SLIDER_X, y, SLIDER_W, SLIDER_H, TFT_BLACK);
  // Label
  tft.setTextDatum(BC_DATUM);
  tft.drawString(String(label) + ": " + String(value, 1), tft.width() / 2, y - 5);
}

void drawSettingsScreen() {
  tft.fillScreen(TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(2);
  tft.drawString("Settings", tft.width() / 2, 10);

  drawSlider(cutInTemp, 16, 30, CUT_TEMP_Y, "Cut-In Temp");
  drawSlider(cutOutTemp, 16, 30, CUT_OUT_Y, "Cut-Out Temp");
  drawSlider(dewOffset, -10, 10, DEW_OFF_Y, "Dew Offset");

  drawButton("Back");
}


// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  dht.begin();
  thermo.begin(MAX31865_2WIRE);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  tft.init();
  tft.setRotation(1);
  ts.begin();
  ts.setRotation(1);

  Serial.println("Setup complete");
}

// -------------------- Loop --------------------
void loop() {
  updateSensors();
  updateRelayLogic();

  if (currentScreen == 1) drawMainScreen();
  else if (currentScreen == 3) drawSettingsScreen();

  for (int i = 0; i < 20; i++) {
    delay(100); // 2 sec refresh cycle total
    if (ts.touched()) {
      TS_Point p = ts.getPoint();

      // Convert raw to screen coordinates (based on your calibration)
      int x = map(p.x, 3812, 556, 0, 320);  // Raw X to Screen X
      int y = map(p.y, 410, 3704, 240, 0);  // Raw Y to Screen Y
      if (currentScreen == 3) {
  if (y >= CUT_TEMP_Y && y <= CUT_TEMP_Y + SLIDER_H) {
    cutInTemp = 16 + ((x - SLIDER_X) / float(SLIDER_W)) * (30 - 16);
    cutInTemp = constrain(cutInTemp, 16, 30);
    drawSettingsScreen();
    break;
  } else if (y >= CUT_OUT_Y && y <= CUT_OUT_Y + SLIDER_H) {
    cutOutTemp = 16 + ((x - SLIDER_X) / float(SLIDER_W)) * (30 - 16);
    cutOutTemp = constrain(cutOutTemp, 16, 30);
    drawSettingsScreen();
    break;
  } else if (y >= DEW_OFF_Y && y <= DEW_OFF_Y + SLIDER_H) {
    dewOffset = -10 + ((x - SLIDER_X) / float(SLIDER_W)) * (10 - (-10));
    dewOffset = constrain(dewOffset, -10, 10);
    drawSettingsScreen();
    break;
  }
}


      Serial.printf("Touch at X=%d Y=%d\n", x, y);

      if (x >= 0 && x < 320 && y >= 0 && y < 240) {
        if (x > BUTTON_X && x < (BUTTON_X + BUTTON_W) &&
            y > BUTTON_Y && y < (BUTTON_Y + BUTTON_H)) {
          currentScreen = (currentScreen == 1) ? 3 : 1;
          Serial.println("Button Pressed: Toggled Screen");
          break;
        }
      }
    }
  }
  
}