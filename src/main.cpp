#include "BluetoothSerial.h"
#include "ConfigData.h"
#include "PatternController.h"

// ==== BUILD CONFIG =====================================================
#define MODE_TFT   1
#define MODE_MOTOR 2

// >>> Switch between MODE_MOTOR or MODE_TFT <<<
#define CONFIG MODE_MOTOR 
// ========================================================================


// ==== HARDWARE SETUP & WIRING ===========================================
#if CONFIG == MODE_TFT
  #include <SPI.h>
  #include <TFT_eSPI.h>
  #include "Renderer/TftActorRenderer.h"
  #include "Display/TftStatusDisplay.h"
  
  TFT_eSPI tft = TFT_eSPI();
  TftActorRenderer renderer(tft);
  TftStatusDisplay statusDisplay(tft);

  // display setup
  void initHardware() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
  }

#elif CONFIG == MODE_MOTOR
  #include "renderer/MotorActorRenderer.h"
  #include "display/NullStatusDisplay.h"

  MotorActorRenderer renderer;
  NullStatusDisplay statusDisplay;

  // motor doesnt need setup
  void initHardware() {} 

#else
  #error "Unknown CONFIG value - please set CONFIG_TFT or CONFIG_MOTOR above"
#endif
// ========================================================================


// ==== GLOBAL OBJECTS ===================================================
BluetoothSerial SerialBT;
PatternController patternController(renderer);


// ==== SETUP =============================================================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("VirboNumber_ESP32");

  initHardware();     // void or display setup
  statusDisplay.begin();
  renderer.begin();   // all off
}


// ==== LOOP ==============================================================
void loop() {
  if (SerialBT.available()) {
    String rawMessage = SerialBT.readStringUntil('\n');

    if (rawMessage.length() > 0) {
      ConfigData receivedConfig(rawMessage);

      if (receivedConfig.isValid) {
        statusDisplay.showConfig(receivedConfig);
        patternController.play(receivedConfig);
      } else {
        statusDisplay.showError();
      }
    }
  }
}