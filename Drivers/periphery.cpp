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
