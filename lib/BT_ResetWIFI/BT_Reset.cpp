#include "Arduino.h"
#include "BT_Reset.h"

BT_ResetWifi::BT_ResetWifi(int pin)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void BT_ResetWifi::resetwifi()
{
  if (digitalRead(_pin) == HIGH)
  {
    Serial.println("reset wifi and restart...!");
    wm.resetSettings();
    ESP.restart();
    digitalWrite(LEDwifi, LOW);
  }
}