#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#ifdef WOKWI_SIMULATION
  #include <Adafruit_ILI9341.h>
#else
  // Echte Hardware
  #include <Adafruit_ST7789.h>
#endif

#include <IRremote.hpp>

// Pins exakt wie in diagram.json verdrahtet
#define TFT_SCLK 13
#define TFT_MOSI 15
#define TFT_CS   5
#define TFT_DC   14
#define TFT_RST  12
#define TFT_MISO -1 // Kein MISO verbunden

#define BTN_A 37
#define BTN_B 39
#define IR_TX_PIN 19
#define IR_RX_PIN 26 

#ifdef WOKWI_SIMULATION
  #define TFT_MISO 36 // Dummy für Wokwi
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);
#else
  // Echte Hardware
  SPIClass fspi(FSPI);
  Adafruit_ST7789 tft = Adafruit_ST7789(&fspi, TFT_CS, TFT_DC, TFT_RST);
#endif

class Knopf {
private:
  int pinNummer;
  bool letzterZustand;
  unsigned long letzteZeit; 

public:
  Knopf(int pin) { //this one recieves the pin number and initializes it temp as pin 
    pinNummer = pin;
    pinMode(pinNummer, INPUT_PULLUP); // PULLUP MEANS THE BUTTON IS HIGH UNTIL I PRESS IT 
    letzterZustand = HIGH;
    letzteZeit = 0;
  }

  
  bool isPressed() {
    bool aktuellerZustand = digitalRead(pinNummer);
    if (aktuellerZustand != letzterZustand) {
      if (millis() - letzteZeit > 50) { // 50 Milli seconds waiting 
        letzteZeit = millis();
        letzterZustand = aktuellerZustand;
        
        if (aktuellerZustand == LOW) { // LOW means gedrueckt (PULLUP)
          return true; 
        }
      }
    }
    return false;
  }
};

// ex..3
class IRDevice {
private:
  uint16_t geraeteAdresse;
  uint8_t anAusCode;

public:
  // const: save the adress and code as constants, they won't change after initialization
  IRDevice(uint16_t adresse, uint8_t code) {
    geraeteAdresse = adresse;
    anAusCode = code;
  }

  // Methode: Infrarot-Befehl aussenden
  void togglePower() {
    // NEC is a common IR protocol, we use it for demonstration.
    IrSender.sendNEC(geraeteAdresse, anAusCode, 0);
  }
};



Knopf knopfA(BTN_A); 
IRDevice meinFernseher(0x0111, 0x12);

void setup() {
  Serial.begin(115200);
  


  #ifdef WOKWI_SIMULATION
  // Simulator Setup
  tft.begin();
  #else
  // Echte Hardware Setup
  fspi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(135, 240); 
  tft.setRotation(1);
  tft.invertDisplay(true); 
  tft.fillScreen(0x0000); 
  #endif

  tft.setTextColor(0xFFFF); 
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("OOP IR-Projekt");
 
  // Infrarot starten
  IrSender.begin(IR_TX_PIN);
}

void loop() {
  //the logic of the program is as follows: we check if the button is pressed, if it is, we send an IR command and update the display.
  
  // is it pressed?
  if (knopfA.isPressed()) {
    
    Serial.println("Knopf A wurde gedrueckt! Sende IR...");
    
    // IR send
    meinFernseher.togglePower();
    
    // (Optional)
    tft.print("IR Gesendet!");
  }
}
