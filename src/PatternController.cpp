#include "PatternController.h"

// Initialize the static array constants with -1 as termination markers
const int PatternController::DIGIT_PATTERNS[10][MAX_PATH_LENGTH] = {
  {0, 1, 2, 5, 8, 7, 6, 3, 0, -1},              // 0
  {1, 4, 7, -1},                                // 1
  {0, 1, 2, 5, 4, 3, 6, 7, 8, -1},              // 2
  {0, 1, 2, 5, 4, 5, 8, 7, 6, -1},              // 3
  {0, 3, 4, 5, 1, 4, 7, -1},                    // 4
  {2, 1, 0, 3, 4, 5, 8, 7, 6, -1},              // 5
  {2, 1, 0, 3, 6, 7, 8, 5, 4, 3, -1},           // 6
  {0, 1, 2, 5, 8, -1},                          // 7
  {1, 0, 3, 4, 5, 8, 7, 6, 3, 4, 5, 2, 1, -1},  // 8
  {2, 1, 0, 3, 4, 5, 2, 5, 8, 7, 6, -1}         // 9
};

PatternController::PatternController(IActorRenderer &renderer) : _renderer(renderer) {}

// Central control hub branching into the specific algorithms
void PatternController::play(const ConfigData &config) {
  if (!config.isValid || config.durationMs <= 0 || config.intensityPct <= 0) {
    _renderer.allOff();
    return;
  }
  
  if (config.mode == "TEST"){
    executeActorTest(config);
  } 
  else if (config.mode == "DISCR") {
    executeDISCR(config);
    delay(config.postDigitDelayMs);
  }
  if (config.mode == "ATM") {
    executeATM(config);
    delay(config.postDigitDelayMs);
  }
  else if (config.mode == "PTS") {
    executePTS(config);
    delay(config.postDigitDelayMs);
  }
  
}


// ==== ALGORITHM 0: ACTORTEST (TEST) =======================================
void PatternController::executeActorTest(const ConfigData &config) {
  int actorIndex        = constrain(config.digit, 0, 8);
  int intensityPct = config.intensityPct;
  int durationMs   = config.durationMs;

  _renderer.allOff();
  _renderer.fire(actorIndex, intensityPct);
  delay(durationMs);
  _renderer.fire(actorIndex, 0);
}


// ==== ALGORITHM 1: DISCRETE (DISCR) =======================================
void PatternController::executeDISCR(const ConfigData &config) {
  int digit        = constrain(config.digit, 0, 9);
  int intensityPct = config.intensityPct;
  int durationMs   = config.durationMs;
  int startHoldFactor   = config.dscrStartHoldFactor;
  int endHoldFactor     = config.dscrEndHoldFactor;

  // 1. Extract the valid pattern sequence and find its length
  int pattern[MAX_PATH_LENGTH];
  int patternSize = 0;
  for (int i = 0; i < MAX_PATH_LENGTH; i++) {
    int actor = DIGIT_PATTERNS[digit][i];
    if (actor == -1) break;
    pattern[patternSize] = actor;
    patternSize++;
  }

  _renderer.allOff();

  for (int i = 0; i < patternSize; i++) {
    int actor = pattern[i];
    _renderer.fire(actor, intensityPct);
    if (i == 0) {
      delay((int)(durationMs * startHoldFactor));
    } else if (i == patternSize - 1) {
      delay((int)(durationMs * endHoldFactor));
    } else {
      delay(durationMs);
    }
    _renderer.fire(actor, 0);
  }
}


// ==== ALGORITHM 2: APPARENT TACTILE MOTION (ATM) =========================
void PatternController::executeATM(const ConfigData &config) {
  int digit        = constrain(config.digit, 0, 9);
  int intensityPct = config.intensityPct;
  int durationMs   = config.durationMs;
  
  // WICHTIG: Faktoren als float behandeln, um 1.5 etc. zu unterstützen
  float startHoldFactor = config.atmStartHoldFactor; 
  float endHoldFactor   = config.atmEndHoldFactor;

  // SOA berechnen
  long soa = (long)(0.32f * durationMs + 47.3f);
  const int frameTimeMs = 5;

  long motorDeadlines[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  int currentActorStates[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

  _renderer.allOff();

  // Muster extrahieren
  int pattern[MAX_PATH_LENGTH];
  int patternSize = 0;
  for (int i = 0; i < MAX_PATH_LENGTH; i++) {
    int actor = DIGIT_PATTERNS[digit][i];
    if (actor == -1) break;
    pattern[patternSize] = actor;
    patternSize++;
  }

  if (patternSize == 0) return;

  // NEU: Lead-in Delay berechnen (Zeit, die der erste Aktor "extra" hält)
  // Wenn Faktor 2.0, dann ist startExtraDelay = 1.0 * duration
  long startExtraDelay = (long)((startHoldFactor - 1.0f) * durationMs);
  if (startExtraDelay < 0) startExtraDelay = 0;

  long startTime = millis();
  long currentTime = 0;
  int nextPatternIndex = 0;

  while (true) {
    currentTime = millis() - startTime;

    bool anyMotorActive = false;
    for (int i = 0; i < 9; i++) {
      if (currentTime < motorDeadlines[i]) {
        anyMotorActive = true;
        break;
      }
    }

    // TRIGGER LOGIK
    if (nextPatternIndex < patternSize) {
      // NEU: OnsetTime Berechnung angepasst
      long targetOnsetTime = (nextPatternIndex == 0) ? 0 : startExtraDelay + (nextPatternIndex * soa);

      if (currentTime >= targetOnsetTime) {
        int motorIndex = pattern[nextPatternIndex];
        
        // Faktor bestimmen
        float currentFactor = 1.0f;
        if (nextPatternIndex == 0) {
          currentFactor = startHoldFactor;
        } else if (nextPatternIndex == patternSize - 1) {
          currentFactor = endHoldFactor;
        }
        
        motorDeadlines[motorIndex] = currentTime + (long)(durationMs * currentFactor);
        nextPatternIndex++;
        anyMotorActive = true; 
      }
    }

    // STATE UPDATE (Hardware-Steuerung)
    for (int i = 0; i < 9; i++) {
      int targetIntensity = (currentTime < motorDeadlines[i]) ? intensityPct : 0;
      if (targetIntensity != currentActorStates[i]) {
        _renderer.fire(i, targetIntensity);
        currentActorStates[i] = targetIntensity;
      }
    }

    // ABBRUCHBEDINGUNG
    if (nextPatternIndex >= patternSize && !anyMotorActive) {
      break;
    }

    delay(frameTimeMs);
  }
}


// ==== ALGORITHM 3: PHANTOM TACTILE SENSATION (PTS) =======================
void PatternController::executePTS(const ConfigData &config) {
  int digit        = constrain(config.digit, 0, 9);
  int intensityPct = config.intensityPct;
  int durationMs   = config.durationMs;
  float startHoldFactor  = config.ptsStartHoldFactor;
  float endHoldFactor    = config.ptsEndHoldFactor;

  const int frameTimeMs = 5;

  _renderer.allOff();

  // 1. Extract the valid pattern sequence and find its length
  int pattern[MAX_PATH_LENGTH];
  int patternSize = 0;
  for (int i = 0; i < MAX_PATH_LENGTH; i++) {
    int actor = DIGIT_PATTERNS[digit][i];
    if (actor == -1) break;
    pattern[patternSize] = actor;
    patternSize++;
  }

  // Safety check: If the pattern is empty, abort
  if (patternSize == 0) return;

  // --- PHASE 1: Start Hold ---
  int firstActor = pattern[0];
  _renderer.fire(firstActor, intensityPct);

  delay((int)(durationMs * startHoldFactor));

  // --- PHASE 2: Linear Pairwise Transitions with Constant Energy Fading ---
  for (int i = 0; i < patternSize - 1; i++) {
    int actor1 = pattern[i];
    int actor2 = pattern[i + 1];

    unsigned long startTime = millis();
    unsigned long elapsedTime = 0;

    int lastA1 = -1;
    int lastA2 = -1;

    while (elapsedTime < (unsigned long)durationMs) {
      float beta = (float)elapsedTime / (float)durationMs;
      if (beta > 1.0f) beta = 1.0f;

      // Constant Energy Formula (Equal-Power Crossfade)
      int a1 = (int)(sqrt(1.0f - beta) * intensityPct);
      int a2 = (int)(sqrt(beta) * intensityPct);

      if (a1 != lastA1) {
        _renderer.fire(actor1, a1);
        lastA1 = a1;
      }

      if (a2 != lastA2) {
        _renderer.fire(actor2, a2);
        lastA2 = a2;
      }

      delay(frameTimeMs);
      elapsedTime = millis() - startTime;
    }

    // Clean up: Explicitly turn off actor1 before moving to the next pair
    _renderer.fire(actor1, 0);
  }

  // --- PHASE 3: End Hold ---
  int lastActor = pattern[patternSize - 1];
  delay((int)(durationMs * endHoldFactor));
  _renderer.fire(lastActor, 0);
}