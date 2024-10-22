#include "periphery.h"


extern TIM_HandleTypeDef htim1;

namespace Periphery
{

    namespace Led
    {
        void SetLed1(GPIO_PinState set)
        {
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,set);
        }
    }

    namespace Encoder
    {
        void SetAutoReload(uint16_t value)
        {
            __HAL_TIM_SetAutoreload(&htim1,value);
        }
    }
}

extern TIM_HandleTypeDef htim2;
namespace Periphery::AddressableLedStrip
{
    void Update(void* data, size_t len)
    {
        HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)data, len);
    }

    void Reset()
    {
        HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_2);
    }
}
