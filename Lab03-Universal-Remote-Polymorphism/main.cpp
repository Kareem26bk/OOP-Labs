#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#ifdef WOKWI_SIMULATION
  #include <Adafruit_ILI9341.h>
#else
  #include <Adafruit_ST7789.h>
#endif

#include <IRremote.hpp>

// --- Pins ---
#define TFT_SCLK 13
#define TFT_MOSI 15
#define TFT_CS   5
#define TFT_DC   14
#define TFT_RST  12
#define TFT_MISO -1

#define BTN_A 37
#define BTN_B 39
#define IR_TX_PIN 19
#define STATUS_LED_PIN 10 // Test pin for the status LED (Task 4)

#ifdef WOKWI_SIMULATION
  #define TFT_MISO 36
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);
#else
  SPIClass fspi(FSPI);
  Adafruit_ST7789 tft = Adafruit_ST7789(&fspi, TFT_CS, TFT_DC, TFT_RST);
#endif

// --- Button Class
class Knopf {
private:
  int _pinNumber;
  bool _lastState;
  unsigned long _lastTime;

public:
  Knopf(int pin) {
    _pinNumber = pin;
    pinMode(_pinNumber, INPUT_PULLUP);
    _lastState = HIGH;
    _lastTime = 0;
  }

  bool isPressed() {
    bool currentState = digitalRead(_pinNumber);
    if (currentState != _lastState) {
      if (millis() - _lastTime > 30) {
        _lastTime = millis();
        _lastState = currentState;
        if (currentState == LOW) return true; 
      }
    }
    return false;
  }
};

// ===================================================================

// 4: RAII helper class for the LED ---
class TransmitLedGuard {
private:
  int _pin;
public:
  // Constructor
  TransmitLedGuard(int pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH); 
  }
  // Destructor: turns off automatically at the end of the block
  ~TransmitLedGuard() {
    digitalWrite(_pin, LOW); 
  }
};

// 2.1: The Interface
class IDisplayable {
public:
  virtual String getDisplayName() const = 0; 
};

// --- 1.1 & 1.2: Abstract Base Class ---
class AbstractDevice {
public:
  virtual void togglePower() = 0; 
};

// ===================================================================

// --- 1.3 & 2.2: TV Device (Multiple Inheritance) ---
class TVDevice : public AbstractDevice, public IDisplayable {
private:
  uint32_t _deviceCode;
  uint32_t _powerOnCode;
  uint32_t _powerOffCode;
  bool _isOn;

public:
  TVDevice(uint32_t deviceCode, uint32_t powerOnCode, uint32_t powerOffCode) {
    _deviceCode = deviceCode;
    _powerOnCode = powerOnCode;
    _powerOffCode = powerOffCode;
    _isOn = false;
  }

  void togglePower() override {
    // Create RAII Guard -> LED goes ON
    TransmitLedGuard guard(STATUS_LED_PIN); 
    
    if (_isOn) {
      IrSender.sendNEC(_deviceCode, _powerOffCode, 0);
      _isOn = false;
    } else {
      IrSender.sendNEC(_deviceCode, _powerOnCode, 0);
      _isOn = true;
    }
    // ends here -> Destructor is called
  }

  String getDisplayName() const override {
    return "TV Device";
  }
};

// 1.3 & 2.2: (AC) 
class ACDevice : public AbstractDevice, public IDisplayable {
private:
  bool _isOn;

public:
  ACDevice() {
    _isOn = false;
  }

  void togglePower() override {
    TransmitLedGuard guard(STATUS_LED_PIN); // LED goes ON
    
    if (_isOn) {
      Serial.println("AC: Sending complex OFF signal...");
      _isOn = false;
    } else {
      Serial.println("AC: Sending complex ON signal...");
      _isOn = true;
    }
    // LED goes OFF automatically here
    //in tis case the actual IR sendingcode is not implemented, but the RAII pattern is still demonstrated with the staus LED.
  }

  String getDisplayName() const override {
    return "Air Conditioner";
  }
};

// ===================================================================

// 2.3: Decouple Display Class 
class StatusDisplay {
private:
  Adafruit_GFX& _tft; 

public:
  StatusDisplay(Adafruit_GFX& display) : _tft(display) {}

  //(The display doesn't know about IR codes anymore!)
  void showDeviceName(const IDisplayable* device) {
    _tft.fillScreen(0x0000); 
    _tft.setCursor(0, 50);
    _tft.setTextColor(0xFFFF); 
    _tft.setTextSize(2);
    _tft.println("Selected Device:");
    
    _tft.setCursor(0, 100);
    _tft.setTextColor(0x07E0); 
    _tft.setTextSize(3);
    _tft.println(device->getDisplayName()); 
  }
};

// ===================================================================

Knopf knopfA(BTN_A); 
Knopf knopfB(BTN_B); //switch devices

TVDevice myTV(0x0111, 0x0A, 0x0B); 
ACDevice myAC; 

// 3.1: Array of pointers to the base class ---
AbstractDevice* devices[2] = { &myTV, &myAC };

// Also an array for the display interface
IDisplayable* displayables[2] = { &myTV, &myAC };

int currentIndex = 0; // 0 = TV, 1 = AC

StatusDisplay myDisplay(tft);                          

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

  IrSender.begin(IR_TX_PIN);

  // Show the first device on the display
  myDisplay.showDeviceName(displayables[currentIndex]); 
}

void loop() {
  // 3.2: Switch with Button B ---
  if (knopfB.isPressed()) {
    
    currentIndex = (currentIndex + 1) % 2; 
    
    // Update display
    myDisplay.showDeviceName(displayables[currentIndex]);
  }

  // 3.3: Switch the current device with Button A 
  if (knopfA.isPressed()) {
    // Polymorphism in action
    devices[currentIndex]->togglePower();
  }
}
