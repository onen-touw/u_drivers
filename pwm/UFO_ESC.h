#pragma once

#include "pwmbs.h"

namespace ufo
{
    namespace drv
    {
        // thread-unsavely
        class UFO_ESC_driver : public pwmbs_t
        {
        public:
            static constexpr uint8_t resolution = 16;
            static constexpr uint8_t frequency = 55;
            static constexpr uint16_t pwm_max_out = 0xffff;                         // 2^16 (65535 or 0xffff)
            static constexpr uint16_t pwm_min_out = 0;                              // 0
            static constexpr uint16_t esc_max_out = pwm_max_out / 10;               // 2^16/10;
            static constexpr uint16_t esc_min_out = esc_max_out/2;                  // 2^16/10/2;

        public:
            UFO_ESC_driver() {}
            ~UFO_ESC_driver() {}

            void setup(uint8_t escID, uint8_t pin)
            {
                _timer = static_cast<ledc_timer_t>(escID % UFO_PWM_TIMER_PER_OUT);
                _channel = static_cast<ledc_channel_t>(escID);
                _pin = pin;

                gpio_config_t conf = {
                    .pin_bit_mask = (1ULL << pin),
                    .mode = GPIO_MODE_DISABLE,
                    .pull_up_en = GPIO_PULLUP_DISABLE,
                    .pull_down_en = GPIO_PULLDOWN_DISABLE,
                    .intr_type = GPIO_INTR_DISABLE,
                };
                if (gpio_config(&conf) != ESP_OK)
                {
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    // CriticalError_t e;
                    // e._info = GenerateInfo_Code(error::codes_t::pwm_gpio_cfg, "GPIO config");
                    // _error.Push(e);
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::pwm_gpio_cfg, "GPIO config")));
                    return;
                }
                
                ledc_timer_config_t timerCfg ={};
                timerCfg.speed_mode = UFO_PWM_TIMER_MODE;
                timerCfg.duty_resolution = static_cast<ledc_timer_bit_t>(resolution);
                timerCfg.timer_num = _timer;
                timerCfg.freq_hz = frequency;
                timerCfg.clk_cfg = UFO_CLK;

                if (ledc_timer_config(&timerCfg) != ESP_OK)
                {
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::pwm_ledc_timer, "ledc_timer config")));
                    return;
                }

                // _duty = ledc_get_duty(UFO_PWM_TIMER_MODE, static_cast<ledc_channel_t>(_channel));
                ledc_channel_config_t chanCfg = {};
                chanCfg.gpio_num = _pin;
                chanCfg.speed_mode = UFO_PWM_TIMER_MODE;
                chanCfg.channel = _channel;
                chanCfg.intr_type = LEDC_INTR_DISABLE;
                chanCfg.timer_sel = _timer;
                chanCfg.duty = _duty;
                chanCfg.hpoint = 0;
                
                if (ledc_channel_config(&chanCfg) != ESP_OK)
                {
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::pwm_ledc_cfg, "ledc_channel config")));
                    return;
                }
            }

            void write(uint16_t val)
            {
                if (_duty == val)
                {
                    return;
                }
                
                _duty = val;
                esp_err_t err = ledc_set_duty(LEDC_HIGH_SPEED_MODE, _channel, _duty);
                err = ledc_update_duty(LEDC_HIGH_SPEED_MODE, _channel);

                if (err != ESP_OK)
                {
                    Trace_t::log("error: ledc_set_duty/ledc_update_duty\n");
                }
            }

            uint16_t get_duty() const
            {
                return _duty;
            }
        };
        
    } // namespace drv

} // namespace ufo
