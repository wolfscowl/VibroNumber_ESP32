#ifndef ISTATUS_DISPLAY_H
#define ISTATUS_DISPLAY_H

#include "../ConfigData.h"

// Interface for TftStatusDisplay & NullStatusDisplay
class IStatusDisplay {
public:
  virtual ~IStatusDisplay() {}

  // initialize the display/state.
  virtual void begin() = 0;

  // Shows the currently receiced configuration.
  virtual void showConfig(const ConfigData &config) = 0;

  // Shows that parsing the received message failed.
  virtual void showError() = 0;
};

#endif