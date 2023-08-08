#ifndef FIXWIFI_H_
#define FIXWIFI_H_

#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>

//global vars needed for fixWiFi()
inline unsigned long WiFifix = 0;
inline unsigned long problemDetected = 0;
inline int problemCount = 0;

void fixWiFi();

#endif /* FIXWIFI_H_ */

#pragma once
