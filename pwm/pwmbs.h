#pragma once

#include "u_sys/config.h"
#include "u_sys/error.h"

// idf-include
#include <driver/ledc.h>

#define UFO_PWM_TIMER_MODE LEDC_HIGH_SPEED_MODE
// #define UFO_PWM_TIMER_GROUP         ((uint8_t)1)
#define UFO_PWM_TIMER_PER_OUT ((uint8_t)2)
#define UFO_CLK LEDC_AUTO_CLK

namespace ufo
{
    namespace drv
    {
        class pwmbs_t
        {
        protected:
            ledc_timer_t _timer = LEDC_TIMER_MAX;
            ledc_channel_t _channel = LEDC_CHANNEL_MAX;
            uint8_t _pin = 255;
            uint16_t _duty = 0;

        public:
            pwmbs_t(/* args */) {}
            ~pwmbs_t() {}
        };

    }
}