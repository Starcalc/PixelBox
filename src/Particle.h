#ifndef PARTICLE_H
#define PARTICLE_H
#include <Arduino.h>

class NewMatrix; // Forward  declaration
class Particle
{
  public:
    Particle(NewMatrix * parent, uint8_t x, uint8_t y, float brightness, float inc = 3, float decay = 0.90);
    Particle();
    bool operator==(const Particle &p) const;
    void update();
    void updatew();
    int _id;
    float brightness();
  private:
    bool increasing = true;
    float _x;
    float _y;
    float _inc;
    float _brightness;
    float _decay;
    NewMatrix * _parent;
};

#endif
