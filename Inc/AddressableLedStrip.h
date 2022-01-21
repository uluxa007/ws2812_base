#ifndef WS2812_BASE_ADDRESSABLELEDSTRIP_H
#define WS2812_BASE_ADDRESSABLELEDSTRIP_H

#include "main.h"
#include "initialize.h"

struct RBGColor
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

class AddressableLedStrip : Common::IVirtualTimerListener
{
public:
    AddressableLedStrip(Common::IVirtualTimer& update_timer);

    AddressableLedStrip& SetPixelColor(RBGColor color,uint16_t x);
    AddressableLedStrip& SetPixelColor(uint8_t r,uint8_t g, uint8_t b,uint16_t x){ SetPixelColor({r,g,b},x);return *this;};
    AddressableLedStrip& SetIntervalColor(RBGColor color,uint16_t x, uint16_t len);
    AddressableLedStrip& SetIntervalColor(uint8_t r,uint8_t g, uint8_t b,uint16_t x, uint16_t len){SetIntervalColor({r,g,b},x,len);return *this;};

    void OnTimer(Common::IVirtualTimer& timer) override;
private:

    void Update();

    LedData m_led_data;

    Common::IVirtualTimer& m_update_timer;
};


#endif //WS2812_BASE_ADDRESSABLELEDSTRIP_H
