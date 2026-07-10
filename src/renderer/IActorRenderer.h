#ifndef IACTOR_RENDERER_H
#define IACTOR_RENDERER_H
 
// Interface for TftActorRenderer & TftMotorActorRender
class IActorRenderer {
public:
  virtual ~IActorRenderer() {}

  // initialises the hardware
  virtual void begin() = 0;
 
  // Turns off all actors
  virtual void allOff() = 0;
 
  // Drives a single actor with the given intensity
  virtual void fire(int actorNumber, int intensity) = 0;
};
 
#endif