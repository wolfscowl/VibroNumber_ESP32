#ifndef CONFIGDATA_H
#define CONFIGDATA_H

#include <Arduino.h>

class ConfigData {
public:
  // Properties with safe default values to prevent memory garbage
  int digit = 0;
  String mode = "";
  int durationMs = 0;
  int intensityPct = 0;
  int postDigitDelayMs = 0;
  float dscrStartHoldFactor = 0.0f;
  float dscrEndHoldFactor = 0.0f;
  float atmStartHoldFactor = 0.0f;
  float atmEndHoldFactor = 0.0f;
  float ptsStartHoldFactor = 0.0f;
  float ptsEndHoldFactor = 0.0f;
  bool isValid = false;

  // 1. Standard constructor (creates an empty, invalid object)
  ConfigData();

  // 2. Intelligent constructor for self-initialization from CSV string
  ConfigData(String rawMessage);
};

#endif