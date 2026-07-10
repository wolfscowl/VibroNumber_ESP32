#ifndef PATTERN_CONTROLLER_H
#define PATTERN_CONTROLLER_H
 
#include <Arduino.h>
#include "ConfigData.h"
#include "renderer/IActorRenderer.h"
 
class PatternController {
private:
  IActorRenderer &_renderer; // Knows only the interface,
 
  // Pattern data structures
  static const int MAX_PATH_LENGTH = 14; // Expanded for termination marker
  static const int DIGIT_PATTERNS[10][MAX_PATH_LENGTH];
 
  // --- Haptic Algorithm Implementations ---

  // 0. ActorTest
  void executeActorTest(const ConfigData &config);
  
  // 1. Basic discrete step-by-step rendering (No overlap)
  void executeDISCR(const ConfigData &config);
 
  // 2. Apparent Tactile Motion (Temporal overlap)
  void executeATM(const ConfigData &config);
 
  // 3. Phantom Tactile Sensation (Dynamic amplitude/intensity fading)
  void executePTS(const ConfigData &config);
 
public:
  // Constructor requires a reference to any IActorRenderer implementation
  PatternController(IActorRenderer &renderer);
 
  // Main entry point to play a pattern based on the received configuration
  void play(const ConfigData &config);
};
 
#endif