#include "c_to_cpp.h"
#include "AddressableLedStrip.h"
#include "initialize.h"

void ws2812_pixel_rgb_to_buf_dma(uint8_t Rpixel , uint8_t Gpixel, uint8_t Bpixel, uint16_t posX)
{
    System::GetAddressableLedStrip()->SetPixelColor(Rpixel,Gpixel,Bpixel,posX);
}

void ws2812_pixel_rgb_to_buf_dma_interval(uint8_t Rpixel , uint8_t Gpixel, uint8_t Bpixel, uint16_t start,uint16_t len)
{
    System::GetAddressableLedStrip()->SetIntervalColor(Rpixel,Gpixel,Bpixel,start,len);
}

void ws2812_light(void)
{

}