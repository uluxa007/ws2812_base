#include "portcritical.h"
#include "main.h"

static uint32_t lock_count = 0;

void __critical_enter(void)
{
    __disable_irq();
    lock_count++;
}

void __critical_exit(void)
{
    if (--lock_count == 0)
        __enable_irq();
}

