#ifndef WS2812_BASE_PERIPHERY_H
#define WS2812_BASE_PERIPHERY_H
#include "main.h"

namespace Periphery
{

    namespace Led
    {
        void SetLed1(GPIO_PinState);
    }

    namespace Encoder
    {
        void SetAutoReload(uint16_t);
    }
}

namespace Periphery::AddressableLedStrip
{
    void Update(void* data, size_t len);
    void Reset();
}
#endif //WS2812_BASE_PERIPHERY_H
