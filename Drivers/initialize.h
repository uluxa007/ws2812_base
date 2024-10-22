#ifndef WS2812_BASE_INITIALIZE_H
#define WS2812_BASE_INITIALIZE_H

#include "main.h"
#include <virtual_timer.h>

using TimerFactory = Common::VirtualTimerFactory<6>;

template<uint16_t LED_COUNT, uint32_t TIMER_FREQUENCY>
class AddressableLedStrip;

using LedStrip = AddressableLedStrip<6, 100000000>;

namespace System
{
    TimerFactory& GetTimerFactory();
    LedStrip* GetAddressableLedStrip();
    void SetAddressableLedStrip(LedStrip* led_strip);

    void Init();
}

#endif //WS2812_BASE_INITIALIZE_H
