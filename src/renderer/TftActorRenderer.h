#ifndef TFT_ACTOR_RENDERER_H
#define TFT_ACTOR_RENDERER_H
 
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "IActorRenderer.h"
 
class TftActorRenderer : public IActorRenderer {
private:
  TFT_eSPI &_tft;
 
  // Grid layout geometry
  const int _gridSize = 3;
  const int _cellSpacing = 30;
  const int _startX = 10;
  const int _startY = 50;
  const int _dotRadius = 10;
 
  uint16_t convertIntensityToColor(int intensity);
 
public:
  TftActorRenderer(TFT_eSPI &tftInstance);
 
  void begin() override;
  void allOff() override;
  void fire(int actorNumber, int intensity) override;
};
 
#endif