#include "AddressableLedStrip.h"

extern TIM_HandleTypeDef htim2;

#define BitIsSet(reg, bit) ((reg & (1<<bit)) != 0)

AddressableLedStrip::AddressableLedStrip(Common::IVirtualTimer& update_timer)
: m_update_timer(update_timer)
{
    m_update_timer.Subscribe(*this);
    m_update_timer.Restart();

    for(int i=DELAY_LEN;i<ARRAY_LEN;i++)
        m_led_data[i] = LOW;
}

AddressableLedStrip& AddressableLedStrip::SetPixelColor(RBGColor color,uint16_t x)
{
    if(x >= LED_COUNT)
        return *this;
    volatile uint16_t i;
#ifdef LOW_POWER
    color.R = color.R / 8;
    color.G = color.G / 8;
    color.B = color.B / 8;
#endif
    for(i = 0;i < 8;i++)
    {
        if (BitIsSet(color.R,(7 - i)) == 1)
        {
            m_led_data[DELAY_LEN + x * 24 + i + 8] = HIGH;
        }else
        {
            m_led_data[DELAY_LEN + x * 24 + i + 8] = LOW;
        }
        if (BitIsSet(color.G,(7 - i)) == 1)
        {
            m_led_data[DELAY_LEN + x * 24 + i + 0] = HIGH;
        }else
        {
            m_led_data[DELAY_LEN + x * 24 + i + 0] = LOW;
        }
        if (BitIsSet(color.B,(7- i)) == 1)
        {
            m_led_data[DELAY_LEN + x * 24 + i + 16] = HIGH;
        }else
        {
            m_led_data[DELAY_LEN + x * 24 + i + 16] = LOW;
        }
    }
    return *this;
}

AddressableLedStrip& AddressableLedStrip::SetIntervalColor(RBGColor color,uint16_t x, uint16_t len)
{
    for(uint16_t i = 0;i < len && (x + i) < LED_COUNT; i++)
    {
        SetPixelColor(color,x + i);
    }
    return *this;
}

void AddressableLedStrip::Update()
{
    HAL_TIM_PWM_Start_DMA(&htim2,TIM_CHANNEL_2,(uint32_t*)m_led_data.data(),ARRAY_LEN);
}

void AddressableLedStrip::OnTimer(Common::IVirtualTimer& timer)
{
    if(timer.GetId() == m_update_timer.GetId())
    {
        Update();
        m_update_timer.Restart();
    }
}

