#include <AddressableLedStrip.h>
#include "initialize.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;

namespace
{
    TimerFactory g_timer_factory;
    LedStrip* g_led_strip;
}

void HAL_TIMEx_CommutationCallback()
{
    g_timer_factory.Decrement(1);
}

namespace System
{
    TimerFactory& GetTimerFactory()
    {
        return g_timer_factory;
    }

    LedStrip* GetAddressableLedStrip()
    {
        return g_led_strip;
    }

    void SetAddressableLedStrip(LedStrip* led_strip)
    {
        g_led_strip = led_strip;
    }

    void Init()
    {
        SystemCoreClockUpdate();
        __HAL_TIM_SET_AUTORELOAD(&htim2,SystemCoreClock/800000 - 1);
        HAL_TIM_Base_Start_IT(&htim5);
    }
}
