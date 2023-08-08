#ifndef ROCKET_H
#define ROCKET_H

class NewMatrix; // Forward  declaration
class Rocket
{
  public:
    int _id;
    Rocket(NewMatrix *parent, float pos, float rocketspeed, float rocket_slowdown);
    Rocket();
    bool operator==(const Rocket &r) const;
    void update();
    int pos();
    float rocketspeed();
    int id();
    uint16_t iteration();
  private:
    float _pos;
    float _speed;
    int _lastbright;
    float _rocket_slowdown;
    uint16_t _iteration;
    NewMatrix * _parent;

};

#endif
