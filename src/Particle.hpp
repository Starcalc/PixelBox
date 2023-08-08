#ifndef PARTICLE_HPP_
#define PARTICLE_HPP_

#include "Particle.h"
#include "NewMatrix.hpp"

Particle::Particle() // Particle::Particle(Adafruit_NeoMatrix * parent)
{
  _x = 0;
  _y = 0;
  // _id = parent->maxParticleID;
  // parent->maxParticleID++;
}  //Default constructor.

Particle::Particle(NewMatrix * parent, uint8_t x, uint8_t y, float brightness, float inc, float decay)
{
  _id = parent->maxParticleID;
  parent->maxParticleID++;
  _x = x;
  _y = y;
  _inc = inc;
  _brightness = brightness;
  _decay = decay;
  _parent = parent;
}

bool Particle::operator==(const Particle &p) const {
  return (p._id == _id);
}

void Particle::update()
{
  if (increasing)
  {
    _brightness *= _inc;
    if (_brightness > 255)
    {
      _brightness = 255;
      increasing = false;
    }
  } else {
    _brightness *= _decay;
  }
  _parent->setMatrixPixelColor((int)_x, (int)_y, _parent->Color((int)_brightness, (int)_brightness, (int)_brightness));
}

void Particle::updatew()
{
  if (increasing)
  {
    _brightness *= _inc;
    if (_brightness > 255)
    {
      _brightness = 255;
      increasing = false;
    }
  } else {
    _brightness *= _decay;
  }
  // _parent->setMatrixPixelColorw((int)_x, (int)_y, (int)_brightness, (int)_brightness, (int)_brightness, (int)_brightness);
  _parent->setMatrixPixelColorw((int)_x, (int)_y, 0,0,0, (int)_brightness);
  // _parent->setMatrixPixelColor((int)_x, (int)_y, _parent->Color((int)_brightness, (int)_brightness, (int)_brightness));
}

float Particle::brightness()
{
  return _brightness;
}
#endif