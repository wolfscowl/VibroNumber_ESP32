#ifndef MOTOR_ACTOR_RENDERER_H
#define MOTOR_ACTOR_RENDERER_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include "IActorRenderer.h"


//
// Wiring:
//   Actors 0-7 -> MUX1 (0x70), channels 0-7
//   Actor  8   -> MUX2 (0x71), channel 0
class MotorActorRenderer : public IActorRenderer {
public:
  MotorActorRenderer();

  void begin() override;
  void allOff() override;
  void fire(int actorNumber, int intensity) override;

  // Diagnostic: for every actor, report whether the multiplexer ACKs and
  // whether the driver (0x5A) ACKs *through* that mux channel. Proves whether
  // the I2C control path (esp. via MUX2) actually works. Prints to Serial.
  void scanBus();

private:
  static const uint8_t DRV_ADDR  = 0x5A; // all DRV2605L share this I2C address
  static const uint8_t MUX1_ADDR = 0x70;
  static const uint8_t MUX2_ADDR = 0x71;

  Adafruit_DRV2605 _drv[9];
  bool _connected[9];

  // Maps actor index (0-8) to the multiplexer address / channel it sits on.
  uint8_t muxAddr(uint8_t i) { return i < 8 ? MUX1_ADDR : MUX2_ADDR; }
  uint8_t muxCh(uint8_t i)   { return i < 8 ? i : 0; }

  // Disables both multiplexers, then enables exactly one channel on one
  // multiplexer. Necessary because all 9 DRV2605L share the same I2C
  // address (0x5A) - only one may be visible on the bus at a time.
  void selectChannel(uint8_t addr, uint8_t ch);
};

#endif
