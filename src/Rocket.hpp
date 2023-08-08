#ifndef ROCKET_HPP_
#define ROCKET_HPP_

#include "Rocket.h"
#include "NewMatrix.hpp"

class NewMatrix; // Forward  declaration

Rocket::Rocket()
{
  //  _id = maxRocketID;
  //  maxRocketID++;
  _pos = 0;
  _speed = 1;
  _lastbright = 1;
}

Rocket::Rocket(NewMatrix *parent, float pos, float rocketspeed, float rocket_slowdown)
{
  _parent = parent;
  _id = _parent->maxRocketID;
  _parent->maxRocketID++;
  _rocket_slowdown = rocket_slowdown;
  _iteration = 0;
  Serial.print("Rocket: ");
  Serial.print(_id);
  Serial.print(" ");
  Serial.print(pos);
  Serial.print(" ");
  Serial.println(rocketspeed);
  _pos = pos;
  _speed = rocketspeed;

}

bool Rocket::operator==(const Rocket &r) const {
  return (r._id == _id);
}

void Rocket::update()
{
  _iteration++;
  _pos += _speed;
  _speed *= _rocket_slowdown; // 0.97
  _parent->setPixelColor(_pos, _parent->Color(50, 32, 0));
}
// Schweif mit Sparkle
int Rocket::pos()
{
  return _pos;
}

float Rocket::rocketspeed()
{
  return _speed;
}

int Rocket::id()
{
  return _id;
}

uint16_t Rocket::iteration()
{
  return _iteration;
}
#endif