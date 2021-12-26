#ifndef WS2812_BASE_C_TO_CPP_H
#define WS2812_BASE_C_TO_CPP_H


#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#endif


void ws2812_pixel_rgb_to_buf_dma(uint8_t Rpixel , uint8_t Gpixel, uint8_t Bpixel, uint16_t posX);
void ws2812_pixel_rgb_to_buf_dma_interval(uint8_t Rpixel , uint8_t Gpixel, uint8_t Bpixel, uint16_t start,uint16_t len);
void ws2812_light(void);

#ifdef __cplusplus
}
#endif


#endif //WS2812_BASE_C_TO_CPP_H
