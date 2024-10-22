#ifndef WS2812_BASE_ADDRESSABLELEDSTRIP_H
#define WS2812_BASE_ADDRESSABLELEDSTRIP_H

#include <cmath>

#include "main.h"
#include "initialize.h"
#include "virtual_timer.h"


struct RGBColor
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
struct HSVColor
{
    uint8_t hue;
    uint8_t saturation;
    uint8_t value;
};

template<uint16_t LED_COUNT, uint32_t TIMER_FREQUENCY>
class AddressableLedStrip : Common::IVirtualTimerListener
{
public:
    AddressableLedStrip(
            Common::IVirtualTimer& update_timer,
            std::function<void(uint32_t* data, uint16_t length)> update_func)
            : m_update_timer(update_timer)
            , m_low_power_mode(false)
            , m_update_func(std::move(update_func))
            , m_led_data{}
            , m_led_color_data{}
            , m_led_on_mask(0)
    {
        m_update_timer.Subscribe(*this);
        m_update_timer.Restart();

        for (int i = DELAY_LEN; i < ARRAY_LEN; i++)
        {
            m_led_data[i] = LOW;
        }
    }

    AddressableLedStrip& SetPixelColor(RGBColor color,uint16_t x)
    {
        if (x >= LED_COUNT)
            return *this;
        m_led_color_data[x] = color;
        return *this;
    }

    AddressableLedStrip& SetIntervalColor(RGBColor color,uint16_t x, uint16_t len)
    {
        for (uint16_t i = 0; i < len && ((x + i) < LED_COUNT); i++)
        {
            SetPixelColor(color, x + i);
        }
        return *this;
    }

    AddressableLedStrip& SetPixelColor(HSVColor color,uint16_t x) { return SetPixelColor(getRGBFromHSV(color), x); };
    AddressableLedStrip& SetIntervalColor(HSVColor color,uint16_t x, uint16_t len) { return SetIntervalColor(getRGBFromHSV(color), x, len); };

    AddressableLedStrip& SetGradientColorHSV(HSVColor start_color, HSVColor end_color, uint16_t start_pos, uint16_t end_pos)
    {
        bool direction  = end_pos > start_pos;
        float positions = fabsf(static_cast<float>(end_pos - start_pos)) + 1;

        auto hue_diff = static_cast<float>(end_color.hue - start_color.hue);
        if (hue_diff < 0)
        {
            hue_diff += 255;
        }
        auto saturation_diff = static_cast<float>(end_color.saturation - start_color.saturation);
        if (saturation_diff < 0)
        {
            saturation_diff += 255;
        }
        auto value_diff = static_cast<float>(end_color.value - start_color.value);
        if (value_diff < 0)
        {
            value_diff += 255;
        }

        float h_step = hue_diff / positions;
        float s_step = saturation_diff / positions;
        float v_step = value_diff / positions;

        HSVColor set_color = start_color;
        if (direction)
        {
            for (uint16_t i = start_pos; i <= end_pos; ++i)
            {
                SetPixelColor(set_color, i);
                set_color.hue        = start_color.hue + h_step * i;
                set_color.saturation = start_color.saturation + s_step * i;
                set_color.value      = start_color.value + v_step * i;
            }
        }
        else
        {
            for (uint16_t i = end_pos; i >= start_pos; --i)
            {
                SetPixelColor(set_color, i);
                set_color.hue        = end_color.hue - h_step * i;
                set_color.saturation = end_color.saturation - s_step * i;
                set_color.value      = end_color.value - v_step * i;
            }
        }

        return *this;
    }

    AddressableLedStrip& SetGradientColorHSVStep(HSVColor start_color, HSVColor step, uint16_t start_pos, uint16_t end_pos)
    {
        HSVColor set_color = start_color;
        for (size_t i = start_pos; i <= end_pos; ++i)
        {
            SetPixelColor(set_color, i);
            set_color.hue += step.hue;
            set_color.saturation += step.saturation;
            set_color.value += step.value;
        }
        return *this;
    }

    AddressableLedStrip& SetGradientColorRGB(RGBColor start_color, RGBColor end_color, uint16_t start_pos, uint16_t end_pos)
    {
        bool direction  = end_pos > start_pos;
        float positions = fabsf(static_cast<float>(end_pos - start_pos)) + 1;

        auto red_diff = static_cast<float>(end_color.red - start_color.red);
        if (red_diff < 0)
        {
            red_diff += 255;
        }
        auto green_diff = static_cast<float>(end_color.green - start_color.green);
        if (green_diff < 0)
        {
            green_diff += 255;
        }
        auto blue_diff = static_cast<float>(end_color.blue - start_color.blue);
        if (blue_diff < 0)
        {
            blue_diff += 255;
        }

        float r_step = red_diff / positions;
        float s_step = green_diff / positions;
        float v_step = blue_diff / positions;

        RGBColor set_color = start_color;
        if (direction)
        {
            for (uint16_t i = start_pos; i <= end_pos; ++i)
            {
                SetPixelColor(set_color, i);
                set_color.red = start_color.red + r_step * i;
                set_color.green = start_color.green + s_step * i;
                set_color.blue = start_color.blue + v_step * i;
            }
        }
        else
        {
            for (uint16_t i = end_pos; i >= start_pos; --i)
            {
                SetPixelColor(set_color, i);
                set_color.red = end_color.red - r_step * i;
                set_color.green = end_color.green - s_step * i;
                set_color.blue = end_color.blue - v_step * i;
            }
        }

        return *this;
    }

    AddressableLedStrip& SetAllPixelColor(RGBColor color) { return SetIntervalColor(color, 0, LED_COUNT); }
    AddressableLedStrip& SetAllPixelColor(HSVColor color) { return SetIntervalColor(color, 0, LED_COUNT); }

    AddressableLedStrip& SetLedOnMask(uint16_t mask) { m_led_on_mask = mask; return *this; };

    AddressableLedStrip& SetAnimationFunction(std::function<void(uint8_t)> animation_function) { m_animation_function = std::move(animation_function); return *this; }

    void SetLowPowerMode(bool value) { m_low_power_mode = value; }
    bool IsLowPowerMode() { return m_low_power_mode; }

    void OnTimer(Common::IVirtualTimer& timer) override
    {
        if (timer.GetId() == m_update_timer.GetId())
        {
            Update();
            m_update_timer.Restart();
        }
    }
private:
    static constexpr uint16_t TIMER_TICK_IN_TACT = TIMER_FREQUENCY / 800000;
    static constexpr uint16_t DELAY_LEN = static_cast<uint16_t >(0.534f * TIMER_TICK_IN_TACT);
    static constexpr uint16_t ARRAY_LEN = DELAY_LEN + LED_COUNT * 24;
    static constexpr uint16_t HIGH = static_cast<uint16_t >(0.72f * TIMER_TICK_IN_TACT);
    static constexpr uint16_t LOW = static_cast<uint16_t >(0.287f * TIMER_TICK_IN_TACT);

    using LedData = std::array<uint32_t, ARRAY_LEN>; // think about date type template
    using LedColorData = std::array<RGBColor, LED_COUNT>;
    using LedOnMask = uint16_t;

    static inline bool BitIsSet(uint8_t reg, uint8_t bit)
    {
        return (reg & (1 << bit)) != 0;
    }

    static uint8_t div255(uint16_t num)
    {
        return (num + 1 + (num >> 8)) >> 8;
    }

    static RGBColor getRGBFromHSV(HSVColor hsv_color)
    {
        RGBColor rgb_color{};
        if (hsv_color.saturation == 0)
        {
            rgb_color.red = rgb_color.green = rgb_color.blue = hsv_color.value;
            return rgb_color;
        }

        const uint8_t region        = (hsv_color.hue * 6) >> 8;
        const int next_region_start = ((((region + 1) << 8) + 5) / 6);  // Can go up to 256, uint8_t not enough
        const uint8_t region_size   = next_region_start - (((region << 8) + 5) / 6);
        const uint8_t remainder     = 255 - (next_region_start - hsv_color.hue) * 255 / region_size;

        const uint8_t p = div255(hsv_color.value * (255 - hsv_color.saturation));
        const uint8_t q = div255(hsv_color.value * (255 - div255(hsv_color.saturation * remainder)));
        const uint8_t t = div255(hsv_color.value * (255 - div255(hsv_color.saturation * (255 - remainder))));

        switch (region)
        {
            case 0:
            {
                rgb_color.red = hsv_color.value, rgb_color.green = t, rgb_color.blue = p;
            }
                break;
            case 1:
            {
                rgb_color.red = q, rgb_color.green = hsv_color.value, rgb_color.blue = p;
            }
                break;
            case 2:
            {
                rgb_color.red = p, rgb_color.green = hsv_color.value, rgb_color.blue = t;
            }
                break;
            case 3:
            {
                rgb_color.red = p, rgb_color.green = q, rgb_color.blue = hsv_color.value;
            }
                break;
            case 4:
            {
                rgb_color.red = t, rgb_color.green = p, rgb_color.blue = hsv_color.value;
            }
                break;
            default:
            {
                rgb_color.red = hsv_color.value, rgb_color.green = p, rgb_color.blue = q;
            }
                break;
        }

        return rgb_color;
    }

    void Update()
    {
        if (m_animation_function)
        {
            static uint8_t animation_counter = 0;
            ++animation_counter;
            m_animation_function(animation_counter);
        }

        for (size_t i = 0; i < LED_COUNT; ++i)
        {
            if (m_led_on_mask & (1 << i))
            {
                UpdatePixelColor(m_led_color_data[i], i);
            }
            else
            {
                UpdatePixelColor(RGBColor{0, 0, 0}, i);
            }
        }
        m_update_func((uint32_t*)m_led_data.data(), ARRAY_LEN);
    }

    void UpdatePixelColor(RGBColor color,uint16_t x)
    {
        if (x >= LED_COUNT)
            return;

        if (m_low_power_mode)
        {
            color.red   = color.red / 8;
            color.green = color.green / 8;
            color.blue  = color.blue / 8;
        }

        for (uint16_t i = 0; i < 8; i++)
        {
            if (BitIsSet(color.red, (7 - i)) == 1)
            {
                m_led_data[DELAY_LEN + x * 24 + i + 8] = HIGH;
            }
            else
            {
                m_led_data[DELAY_LEN + x * 24 + i + 8] = LOW;
            }
            if (BitIsSet(color.green, (7 - i)) == 1)
            {
                m_led_data[DELAY_LEN + x * 24 + i + 0] = HIGH;
            }
            else
            {
                m_led_data[DELAY_LEN + x * 24 + i + 0] = LOW;
            }
            if (BitIsSet(color.blue, (7 - i)) == 1)
            {
                m_led_data[DELAY_LEN + x * 24 + i + 16] = HIGH;
            }
            else
            {
                m_led_data[DELAY_LEN + x * 24 + i + 16] = LOW;
            }
        }
    }

    LedData m_led_data;
    LedColorData m_led_color_data;
    Common::IVirtualTimer& m_update_timer;
    bool m_low_power_mode;
    std::function<void(uint32_t* data, uint16_t length)> m_update_func;
    std::function<void(uint8_t)> m_animation_function;
    LedOnMask m_led_on_mask;
};


#endif //WS2812_BASE_ADDRESSABLELEDSTRIP_H
