#include <SPI.h>
#include <TFT_eSPI.h>
#include "BluetoothSerial.h"
#include "ConfigData.h"
#include "PatternController.h"
#include "Renderer/IActorRenderer.h"
#include "Display/IStatusDisplay.h"

// ==== BUILD CONFIG =====================================================
// Available hardware configurations
#define MODE_TFT   1
#define CONFIG_MOTOR 2

// >>> Change this single line to switch hardware <<<
#define CONFIG CONFIG_MOTOR

// Only include and compile the code for the currently selected CONFIG.
#if CONFIG == MODE_TFT
  #include "Renderer/TftActorRenderer.h"
  #include "Display/TftStatusDisplay.h"
#elif CONFIG == CONFIG_MOTOR
  #include "Renderer/MotorActorRenderer.h"
  #include "Display/NullStatusDisplay.h"
#else
  #error "Unknown CONFIG value - please set CONFIG_TFT or CONFIG_MOTOR above"
#endif
// ========================================================================

BluetoothSerial SerialBT;

// ==== WIRING ============================================================
// Objects are created and wired together here (dependency injection).

// The TFT_eSPI object is only needed in TFT mode.
#if CONFIG == MODE_TFT
TFT_eSPI tft = TFT_eSPI();
#endif

// Concrete implementations, chosen at compile time via CONFIG.
#if CONFIG == MODE_TFT
  TftActorRenderer renderer(tft);
  TftStatusDisplay statusDisplay(tft);
#elif CONFIG == CONFIG_MOTOR
  MotorActorRenderer renderer;
  NullStatusDisplay statusDisplay;
#endif

// PatternController only knows the interfaces, not the concrete classes.
PatternController patternController(renderer);

// ==== SETUP ===========================================================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("VirboNumber_ESP32");

#if CONFIG == MODE_TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
#endif

  statusDisplay.begin();
  renderer.begin(); // draws/sets the initial "all off" state itself
}


// ==== LOOP =============================================================
void loop() {
  if (SerialBT.available()) {
    String rawMessage = SerialBT.readStringUntil('\n');

    if (rawMessage.length() > 0) {
      ConfigData receivedConfig(rawMessage);

      if (receivedConfig.isValid) {
        // 1. Show the active configuration  => only CONFIG_TFT
        statusDisplay.showConfig(receivedConfig);

        // 2. Play the pattern on the 3x3 grid.
        patternController.play(receivedConfig);
      } else {
		    // Show the Error => only CONFIG_TFT 
        statusDisplay.showError();
      }
    }
  }
}
