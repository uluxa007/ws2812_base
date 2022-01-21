#ifndef WS2812_BASE_INITIALIZE_H
#define WS2812_BASE_INITIALIZE_H

#include "main.h"
#include <virtual_timer.h>

constexpr uint32_t DELAY_LEN = 48;
constexpr uint32_t HIGH = 72;
constexpr uint32_t LOW = 28;
constexpr uint32_t LED_COUNT = 300;
constexpr uint32_t ARRAY_LEN = DELAY_LEN + LED_COUNT*24;

using TimerFactory = Common::VirtualTimerFactory<6>;
using LedData = std::array<uint32_t,ARRAY_LEN>;

class AddressableLedStrip;

namespace System
{
    TimerFactory& GetTimerFactory();
    AddressableLedStrip* GetAddressableLedStrip();
    void SetAddressableLedStrip(AddressableLedStrip* led_strip);

    void Init();
}

#endif //WS2812_BASE_INITIALIZE_H
