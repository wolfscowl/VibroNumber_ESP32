#ifndef NULL_STATUS_DISPLAY_H
#define NULL_STATUS_DISPLAY_H

#include "IStatusDisplay.h"

// "Null Object": fulfills the interface but deliberately does nothing.
// Used in motor mode, where there is no display. This way main.ino can
// keep calling statusDisplay.begin()/showConfig()/showError() as usual,
// without if-checks or #ifdef blocks.
class NullStatusDisplay : public IStatusDisplay {
public:
  void begin() override {}
  void showConfig(const ConfigData &config) override {}
  void showError() override {}
};

#endif