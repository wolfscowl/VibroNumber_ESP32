#ifndef TFT_STATUS_DISPLAY_H
#define TFT_STATUS_DISPLAY_H
 
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "IStatusDisplay.h"
 
// Zeigt die Konfiguration/Fehler als Text auf dem TFT-Display an.
// (vorher: initDisplay(), displayData(), displayError() in main.ino)
class TftStatusDisplay : public IStatusDisplay {
private:
  TFT_eSPI &_tft;

public:
  TftStatusDisplay(TFT_eSPI &tftInstance);

  void begin() override;
  void showConfig(const ConfigData &config) override;
  void showError() override;
};

#endif