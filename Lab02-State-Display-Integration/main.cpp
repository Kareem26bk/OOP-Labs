#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#ifdef WOKWI_SIMULATION
  #include <Adafruit_ILI9341.h>
#else
  // Real hardware
  #include <Adafruit_ST7789.h>
#endif

#include <IRremote.hpp>

// Pins wired exactly as in diagram.json
#define TFT_SCLK 13
#define TFT_MOSI 15
#define TFT_CS   5
#define TFT_DC   14
#define TFT_RST  12
#define TFT_MISO -1 // No MISO connected

#define BTN_A 37
#define BTN_B 39
#define IR_TX_PIN 19
#define IR_RX_PIN 26 

#ifdef WOKWI_SIMULATION
  #define TFT_MISO 36 // Dummy for Wokwi
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);
#else
  // Real hardware
  SPIClass fspi(FSPI);
  Adafruit_ST7789 tft = Adafruit_ST7789(&fspi, TFT_CS, TFT_DC, TFT_RST);
#endif

// ===================================================================

// Button Class 
class Knopf {
private:
  int _pinNummer;
  bool _letzterZustand;
  unsigned long _letzteZeit;

public:
  Knopf(int pin) {
    _pinNummer = pin;
    pinMode(_pinNummer, INPUT_PULLUP);
    _letzterZustand = HIGH;
    _letzteZeit = 0;
  }

  bool isPressed() {
    bool aktuellerZustand = digitalRead(_pinNummer);
    if (aktuellerZustand != _letzterZustand) {
      if (millis() - _letzteZeit > 50) {
        _letzteZeit = millis();
        _letzterZustand = aktuellerZustand;
        if (aktuellerZustand == LOW) return true; 
      }
    }
    return false;
  }
};

// --- Task 1 & 2: Extend IRDevice Class ---
class IRDevice {
private:
  // Task 1.1 & 2.1: Private member variables
  uint32_t _deviceCode;
  uint32_t _powerOnCode;
  uint32_t _powerOffCode;
  bool _isOn; // Status of the device

public:
  // Task 1.2: Constructor takes all required values
  IRDevice(uint32_t deviceCode, uint32_t powerOnCode, uint32_t powerOffCode) {
    _deviceCode = deviceCode;
    _powerOnCode = powerOnCode;
    _powerOffCode = powerOffCode;
    _isOn = false; // Always starts in the OFF state
  }

  // Task 1.3: Implement setter methods
  void setDeviceCode(uint32_t code) { _deviceCode = code; }
  void setPowerCodes(uint32_t onCode, uint32_t offCode) {
    _powerOnCode = onCode;
    _powerOffCode = offCode;
  }
  
  // Task 2.3: Setter for status
  void setOn(bool on) { _isOn = on; }

  // Task 1.4 & 2.3: Implement getter methods
  uint32_t getDeviceCode() const { return _deviceCode; }
  uint32_t getPowerOnCode() const { return _powerOnCode; }
  uint32_t getPowerOffCode() const { return _powerOffCode; }
  bool isOn() const { return _isOn; }

  // Task 2.2: Adapt togglePower() logic
  void togglePower() {
    if (_isOn) {
      // Is ON, so send OFF code
      IrSender.sendNEC(_deviceCode, _powerOffCode, 0);
      _isOn = false; // Update status
    } else {
      // Is OFF, so send ON code
      IrSender.sendNEC(_deviceCode, _powerOnCode, 0);
      _isOn = true; // Update status
    }
  }
};

// --- Task 3: Display Class ---
class StatusDisplay {
private:
  // Reference to the base display object (Works for Wokwi AND Hardware)
  Adafruit_GFX& _tft; 

public:
  // Constructor receives display reference (Task 3.1)
  StatusDisplay(Adafruit_GFX& display) : _tft(display) {}

  // Method to display the codes (Task 3.2)
  void showCodes(const IRDevice& device) {
    _tft.fillScreen(0x0000); // Black
    _tft.setCursor(0, 10);
    _tft.setTextColor(0xFFFF); // White
    _tft.setTextSize(2);
    _tft.println("IR Codes:");
    
    // HEX output for clear presentation
    _tft.print("Dev: 0x"); _tft.println(device.getDeviceCode(), HEX);
    _tft.print("ON : 0x"); _tft.println(device.getPowerOnCode(), HEX);
    _tft.print("OFF: 0x"); _tft.println(device.getPowerOffCode(), HEX);
  }

  // Optional method to display the status (Task 3.3)
  void showStatus(const IRDevice& device) {
    // Clear only the lower area to keep the codes
    _tft.fillRect(0, 100, 240, 50, 0x0000); 
    _tft.setCursor(0, 100);
    _tft.setTextSize(3);

    if (device.isOn()) {
      _tft.setTextColor(0x07E0); // Green for ON
      _tft.println("Status: AN");
    } else {
      _tft.setTextColor(0xF800); // Red for OFF
      _tft.println("Status: AUS");
    }
  }
};

// =================================================================== 

// --- Task 4.1: Instantiate objects ---
Knopf knopfA(BTN_A); 

// IRDevice with test values for Device, ON, and OFF code
IRDevice meinFernseher(0x0111, 0x0A, 0x0B); 

// StatusDisplay gets the global 'tft' object passed to it
StatusDisplay meinDisplay(tft); 

// ===================================================================

void setup() {
  Serial.begin(115200);

  #ifdef WOKWI_SIMULATION
  tft.begin();
  #else
  fspi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(135, 240); 
  tft.setRotation(1);
  tft.invertDisplay(true); 
  tft.fillScreen(0x0000); 
  #endif

  // Start Infrared
  IrSender.begin(IR_TX_PIN);

  // Task 4.2: Initial display of the codes
  meinDisplay.showCodes(meinFernseher);
  meinDisplay.showStatus(meinFernseher); // Initially shows "OFF"
}

void loop() {
  // Task 4.3: Check button
  if (knopfA.isPressed()) {
    
    // Call IR logic (Sends code and toggles _isOn)
    meinFernseher.togglePower();
    
    // Task 2.4: Output status in Serial Monitor
    if (meinFernseher.isOn()) {
      Serial.println("TV: AN");
    } else {
      Serial.println("TV: AUS");
    }

    // Task 4.3: Make the new status visible on the display
    meinDisplay.showStatus(meinFernseher);
  }
}
