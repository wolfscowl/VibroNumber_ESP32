#include "MotorActorRenderer.h"

// set all motors to not connected 
MotorActorRenderer::MotorActorRenderer() {
  for (uint8_t i = 0; i < 9; i++) _connected[i] = false;
}

void MotorActorRenderer::selectChannel(uint8_t addr, uint8_t ch) {
  // Disable all channels on both multiplexers first.
  Wire.beginTransmission(MUX1_ADDR); Wire.write(0); Wire.endTransmission();
  Wire.beginTransmission(MUX2_ADDR); Wire.write(0); Wire.endTransmission();

  // Then enable exactly the one channel we actually want to talk to.
  Wire.beginTransmission(addr); Wire.write(1 << ch); Wire.endTransmission();
}

// Initializes all 9 DRV2605L drivers, one at a time via the multiplexers.
void MotorActorRenderer::begin() {
  Wire.begin();

  for (uint8_t i = 0; i < 9; i++) {
    selectChannel(muxAddr(i), muxCh(i));
    delay(10); // let the mux settle

    _connected[i] = _drv[i].begin();
    if (_connected[i]) {
      _drv[i].useLRA();
      _drv[i].setMode(DRV2605_MODE_REALTIME);
    }
    Serial.printf("Actor %d %s\n", i, _connected[i] ? "ready" : "not found!");
  }

  // Make sure every actor starts in a defined, safe "off" state.
  allOff();
}

// Turns off all 9 actors (formerly: displayGrid / ActorArray::allOff).
void MotorActorRenderer::allOff() {
  for (uint8_t i = 0; i < 9; i++) fire(i, 0);
}

// Drives a single actor with the given intensity (formerly: displayActor /
// ActorArray::fire).
//
// IMPORTANT / WICHTIG: The Adafruit_DRV2605 library uses SIGNED 8-bit
// values in RTP mode by default. Values above 127 are interpreted as
// negative and cause erratic behavior. That's why we map the incoming
// 0-100 percentage to 0-127, not to 0-255.
void MotorActorRenderer::fire(int actorNumber, int intensity) {
  if (actorNumber < 0 || actorNumber > 8) return;
  if (!_connected[actorNumber]) return;

  intensity = constrain(intensity, 0, 100);
  uint8_t rtpValue = map(intensity, 0, 100, 0, 127);

  selectChannel(muxAddr(actorNumber), muxCh(actorNumber));
  _drv[actorNumber].setRealtimeValue(rtpValue);
}
