#ifndef BT_ResetWifi
#define BT_ResetWifi
#include "Arduino.h"

class BT_ResetWifi
{
  public:
    BT_ResetWifi(int pin);
    void resetwifi();
  private:
    int _pin;
};

#endif