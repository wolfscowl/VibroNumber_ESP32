#include "MotorActorRenderer.h"

// A/B TEST SWITCH:
//   1 = LRA open-loop (no back-EMF tracking -> silent at rest, but fixed drive
//       frequency; a motor may feel weak if its resonance != OL_LRA_PERIOD).
//   0 = LRA closed-loop (self-tunes to resonance -> strong, but can "hunt" and
//       buzz at rest on marginal channels).
// Flash once with 1 and once with 0 to see which behaviour a motor has.
#define USE_OPEN_LOOP 1

// LIVE CONNECTION TESTER (multimeter substitute):
// Set to an actor index (e.g. 8) to make begin() loop forever, probing that
// actor's I2C link ~5x/sec and printing ACK/NO-ACK. Wiggle the connectors and
// cables of that driver board - when the reading flips, you found the bad
// contact. Set back to -1 for normal operation.
#define LIVE_LINK_TEST_ACTOR -1

// DRV2605L open-loop LRA period register (not defined by the Adafruit lib).
// Drive frequency = 1 / (value * 98.46 us). See table below for typical LRAs.
#define DRV2605_REG_OL_LRA_PERIOD 0x20
static const uint8_t OL_LRA_PERIOD_VALUE = 0x33; // ~200 Hz (chip default)

// set all motors to not connected
MotorActorRenderer::MotorActorRenderer() {
  for (uint8_t i = 0; i < 9; i++) _connected[i] = false;
}

// Diagnostic: check the I2C control path for every actor. For each one it
// verifies (a) the multiplexer itself ACKs on the upstream bus, and (b) the
// driver at 0x5A ACKs *through* the selected mux channel. If MUX2's actor shows
// "mux ACK" but "driver no-ACK", the control path via MUX2 is broken (pull-ups,
// address strap, RESET pin or wiring) - not the firmware, not the motor.
void MotorActorRenderer::scanBus() {
  Serial.println(F("--- I2C control-path scan (mux ACK / driver ACK) ---"));
  for (uint8_t i = 0; i < 9; i++) {
    uint8_t addr = muxAddr(i);
    uint8_t ch   = muxCh(i);

    Wire.beginTransmission(addr);
    bool muxAck = (Wire.endTransmission() == 0);

    selectChannel(addr, ch);
    delay(5);

    Wire.beginTransmission(DRV_ADDR);
    bool drvAck = (Wire.endTransmission() == 0);

    Serial.printf("Actor %d  mux 0x%02X ch%u: %-6s  driver 0x5A: %s\n",
                  i, addr, ch,
                  muxAck ? "ACK" : "NO-ACK",
                  drvAck ? "ACK" : "NO-ACK");
  }
  Serial.println(F("---------------------------------------------------"));
}

void MotorActorRenderer::selectChannel(uint8_t addr, uint8_t ch) {
  // Disable all channels on both multiplexers first.
  Wire.beginTransmission(MUX1_ADDR); Wire.write(0); Wire.endTransmission();
  Wire.beginTransmission(MUX2_ADDR); Wire.write(0); Wire.endTransmission();

  // Then enable exactly the one channel we actually want to talk to.
  Wire.beginTransmission(addr); Wire.write(1 << ch); Wire.endTransmission();
}

// Initializes all 9 DRV2605L drivers, one at a time via the multiplexers.
//
// STARTUP TWITCH / EINSCHALT-ZUCKEN:
// A DRV2605L comes out of standby during begin(), and switching the feedback
// register with useLRA() are both moments where the H-bridge can briefly emit
// an undefined value -> the LRA twitches even though we never asked it to.
// To avoid that we force the real-time value to 0 *immediately* per driver,
// before AND after the LRA feedback switch, instead of relying on the final
// allOff() (which only runs once all 9 are already active).
void MotorActorRenderer::begin() {
  Wire.begin();

  // Print the I2C control-path map first, so you can see straight away whether
  // the driver is reachable through each mux (especially MUX2 / actor 8).
  scanBus();

  // Optional live link tester: continuously probe one actor's I2C connection
  // so you can find a loose contact by wiggling cables (no multimeter needed).
#if LIVE_LINK_TEST_ACTOR >= 0
  Serial.printf("\n>>> LIVE LINK TEST on actor %d - wiggle its cables now <<<\n",
                LIVE_LINK_TEST_ACTOR);
  while (true) {
    selectChannel(muxAddr(LIVE_LINK_TEST_ACTOR), muxCh(LIVE_LINK_TEST_ACTOR));
    Wire.beginTransmission(DRV_ADDR);
    bool ok = (Wire.endTransmission() == 0);
    Serial.printf("actor %d link: %s\n",
                  LIVE_LINK_TEST_ACTOR, ok ? "ACK  (good)" : "NO-ACK  (bad!)");
    delay(200);
  }
#endif

  for (uint8_t i = 0; i < 9; i++) {
    selectChannel(muxAddr(i), muxCh(i));
    delay(10); // let the mux settle

    _connected[i] = _drv[i].begin();
    if (!_connected[i]) {
      Serial.printf("Actor %d not found!\n", i);
      continue;
    }

    // Enter RTP mode and clamp the output to 0 straight away, so the driver
    // never holds a stray drive value while we finish configuring it.
    _drv[i].setMode(DRV2605_MODE_REALTIME);
    _drv[i].setRealtimeValue(0);

    // Switch to LRA feedback, then re-clamp to 0 (the register switch itself
    // can glitch the bridge for a moment).
    _drv[i].useLRA();
    _drv[i].setRealtimeValue(0);

    // OPEN-LOOP LRA / SELBSTVIBRATION IM RUHEZUSTAND:
    // In closed-loop mode the DRV2605L constantly senses the LRA back-EMF to
    // track resonance. On marginal channels that loop can "hunt" and keep
    // driving faintly even at RTP=0 -> the actor buzzes with no input. Running
    // the LRA open-loop removes the back-EMF sensing, so RTP=0 is truly silent.
    // Trade-off: slightly less punch/efficiency, but fully predictable.
#if USE_OPEN_LOOP
    uint8_t ctrl3 = _drv[i].readRegister8(DRV2605_REG_CONTROL3);
    _drv[i].writeRegister8(DRV2605_REG_CONTROL3, ctrl3 | 0x01); // set LRA_OPEN_LOOP

    // Open-loop drive frequency = 1 / (OL_LRA_PERIOD * 98.46 us).
    // 0x33 ~= 200 Hz (chip default). Tune to YOUR LRA's resonance for full
    // strength:  0x3A ~= 175 Hz,  0x33 ~= 200 Hz,  0x2B ~= 235 Hz.
    _drv[i].writeRegister8(DRV2605_REG_OL_LRA_PERIOD, OL_LRA_PERIOD_VALUE);
#endif

    _drv[i].setRealtimeValue(0);

    Serial.printf("Actor %d ready\n", i);
  }

  // Belt-and-suspenders: make sure every actor sits in a defined "off" state.
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
