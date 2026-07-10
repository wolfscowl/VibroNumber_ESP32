#include "TftActorRenderer.h"
 
TftActorRenderer::TftActorRenderer(TFT_eSPI &tftInstance) : _tft(tftInstance) {}
 
// draw the actor grid's background and the "all off" state of all actors
void TftActorRenderer::begin() {
  allOff();
}

// Turns off all actors
void TftActorRenderer::allOff() {
  for (int i = 0; i < 9; i++) fire(i, 0);
}
 
// Drives a single actor with the given intensity
void TftActorRenderer::fire(int actorNumber, int intensity) {
  actorNumber = constrain(actorNumber, 0, 8);
  intensity = constrain(intensity, 0, 100);
  int col = actorNumber % _gridSize;
  int row = actorNumber / _gridSize;
  int x = _startX + (col * _cellSpacing);
  int y = _startY + (row * _cellSpacing);
  _tft.fillCircle(x, y, _dotRadius, convertIntensityToColor(intensity));
}
 

uint16_t TftActorRenderer::convertIntensityToColor(int intensity) {
  if (intensity == 0) return TFT_WHITE;
  int red   = map(intensity, 1, 100, 150, 0);
  int green = map(intensity, 1, 100, 200, 0);
  int blue  = map(intensity, 1, 100, 255, 180);
  return _tft.color565(red, green, blue);
}