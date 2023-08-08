#include "fixWiFi.h"

void fixWiFi() {
  // Posts every 10 seconds the state of WiFi.status(), Homie.getMqttClient().connected() and Homie.isConfigured()
  // Within this interval the connectivity is checked and logged if a problem is detected
  // Then it disconnects Wifi, if Wifi or MQTT is not connected for 1 Minute (but only if Homie is configured)
  if ( WiFifix == 0 || ((millis() - WiFifix) > 10000)) {
    if (Homie.isConfigured()) {
      float rssi = WiFi.RSSI();
      Homie.getLogger() << endl;
      Homie.getLogger() << "Wifi-state:" << WiFi.status() << " | Wi-Fi signal:" <<
        rssi << " | MQTT-state:" << Homie.getMqttClient().connected() << " | HomieConfig-state:" <<
        Homie.isConfigured() << " | freeHeap:" << ESP.getFreeHeap() << endl;
      
      if (!Homie.getMqttClient().connected() || WiFi.status() != 3) {
        if (0 == problemDetected) {
          if (WiFi.status() != 3) {
            Homie.getLogger() << "Connectivity in problematic state --> WiFi: Disconnected " << endl;
          }
          if (!Homie.getMqttClient().connected()) {
            Homie.getLogger() << "Connectivity in problematic state --> MQTT: Disconnected " << endl;
          }
          problemDetected = millis();
        }
        else if ((millis() - problemDetected) > 120000 && (problemCount >= 5)) {
          Homie.getLogger() << "Connectivity in problematic state --> This remained for 10 minutes. Rebooting!" << endl;
          Homie.reboot();
        }
        else if ((millis() - problemDetected) > 120000 && problemCount < 5) {
          problemCount = (problemCount + 1);
          Homie.getLogger() << "Connectivity in problematic state --> " << "/n This remained for 2 minutes. Disconnecting WiFi to start over." << endl;
          problemDetected = 0;
          WiFi.disconnect();
        }
      }
      else if (problemCount != 0 && (Homie.getMqttClient().connected() || WiFi.status() == 3)) {
        problemCount = 0;
        ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
        ArduinoOTA.begin();
      }
    }
    WiFifix = millis();
  }
}
