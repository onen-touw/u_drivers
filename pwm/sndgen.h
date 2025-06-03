#pragma once

#include "pwmbs.h"

namespace ufo
{
    namespace drv
    {
        class sndgen_t : public pwmbs_t
        {
        public:
            static constexpr uint8_t resolution = 10;

        public:
            sndgen_t() {}

            void setup(uint8_t pin)
            {
                _timer = ledc_timer_t::LEDC_TIMER_3;
                _channel = ledc_channel_t::LEDC_CHANNEL_5;
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
                timerCfg.freq_hz = 525;
                timerCfg.clk_cfg = UFO_CLK;

                if (ledc_timer_config(&timerCfg) != ESP_OK)
                {
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::pwm_ledc_timer, "ledc_timer config")));
                    return;
                }

                ledc_channel_config_t chanCfg = {};
                chanCfg.gpio_num = _pin;
                chanCfg.speed_mode = LEDC_LOW_SPEED_MODE;
                chanCfg.channel = _channel;
                chanCfg.intr_type = LEDC_INTR_DISABLE;
                chanCfg.timer_sel = _timer;
                chanCfg.duty = _duty;
                chanCfg.hpoint = 0;
                
                _duty = 0x1FF;

                if (ledc_channel_config(&chanCfg) != ESP_OK)
                {
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::pwm_ledc_cfg, "ledc_channel config")));
                    return;
                }
            }
            ~sndgen_t() {}

            void write(uint16_t freq){
                esp_err_t err = ESP_OK;
                if (freq)
                {
                    ledc_timer_config_t timerCfg ={};
                    timerCfg.speed_mode = LEDC_LOW_SPEED_MODE;
                    timerCfg.duty_resolution = static_cast<ledc_timer_bit_t>(resolution);
                    timerCfg.timer_num = _timer;
                    timerCfg.freq_hz = freq;
                    timerCfg.clk_cfg = UFO_CLK;
    
                    if (ledc_timer_config(&timerCfg) != ESP_OK)
                    {
                        Trace_t::log("ledcWriteTone configuration failed!");
                        return;
                    }
                    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, _channel, _duty);
                    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, _channel);
                }
                else
                {
                    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, _channel, 0);
                    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, _channel);
                }
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

    }
}

/* 
uint32_t ledcWriteTone(uint8_t pin, uint32_t freq) {
  ledc_channel_handle_t *bus = (ledc_channel_handle_t *)perimanGetPinBus(pin, ESP32_BUS_TYPE_LEDC);
  if (bus != NULL) {

    if (!freq) {
      ledcWrite(pin, 0);
      return 0;
    }

    uint8_t group = (bus->channel / 8), timer = ((bus->channel / 2) % 4);

    ledc_timer_config_t ledc_timer = {.speed_mode = group, .timer_num = timer, .duty_resolution = 10, .freq_hz = freq, .clk_cfg = clock_source};

    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
      log_e("ledcWriteTone configuration failed!");
      return 0;
    }
    bus->channel_resolution = 10;

    uint32_t res_freq = ledc_get_freq(group, timer);
    ledcWrite(pin, 0x1FF);
    return res_freq;
  }
  return 0;
} */