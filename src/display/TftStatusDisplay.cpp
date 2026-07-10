#include "TftStatusDisplay.h"
 
TftStatusDisplay::TftStatusDisplay(TFT_eSPI &tftInstance) : _tft(tftInstance) {}
 

void TftStatusDisplay::begin() {
  _tft.setTextColor(TFT_WHITE, TFT_BLACK);
  _tft.setTextSize(2);
  _tft.drawString("Grid Simulation", 0, 10);
}
 
// Shows the currently receiced configuration.
void TftStatusDisplay::showConfig(const ConfigData &config) {
  _tft.fillRect(100, 40, 220, 90, TFT_BLACK);
  _tft.setTextColor(TFT_WHITE, TFT_BLACK);
  _tft.setTextSize(1);
  _tft.drawString("Digit:           " + String(config.digit), 100, 40);
  _tft.drawString("Mode:            " + config.mode, 100, 55);
  _tft.drawString("Duration:        " + String(config.durationMs) + "ms", 100, 70);
  _tft.drawString("Intensity:       " + String(config.intensityPct) + "%", 100, 85);
  _tft.drawString("PTS-StartFactor: " + String(config.ptsStartHoldFactor), 100, 100);
  _tft.drawString("PTS-EndFactor:   " + String(config.ptsEndHoldFactor), 100, 115);
}

// Shows that parsing the received message failed.
void TftStatusDisplay::showError() {
  _tft.fillRect(100, 40, 220, 90, TFT_BLACK);
  _tft.setTextColor(TFT_RED, TFT_BLACK);
  _tft.setTextSize(1);
  _tft.drawString("Parsing Fehler!", 100, 40);
}