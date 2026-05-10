#pragma once

#include "u_driver_spiffs_cfg.h"
#include "esp_log.h"

namespace __u_drivers
{
    class u_driver_spiffs_t
    {
    private:
        static constexpr const char* tag = "spfs_drv";
        bool _initialized = false;
        
    public:

        u_driver_spiffs_t() 
        {
            
        }
        ~u_driver_spiffs_t() 
        {
           driver_deinit();
        }


        void driver_init(const driver_spiffs_cnf_t& cfg)
        {

            if (_initialized)
            {
                return;
            }

            __meta_spiffs.set_state(driver_state_t::initialized);

            ESP_LOGI(tag, "[init]: Start");

            esp_err_t ret = esp_vfs_spiffs_register(&cfg);
            if (ret != ESP_OK)
            {
                if (ret == ESP_FAIL)
                {
                    __set_err(1);
                    ESP_LOGE(tag,"[init]: Failed to mount or format filesystem");
                }
                else if (ret == ESP_ERR_NOT_FOUND)
                {
                    __set_err(2);
                    ESP_LOGE(tag, "[init]: Failed to find SPIFFS partition");
                }
                else
                {
                    __set_err(3);
                    ESP_LOGE(tag, "[init]: Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
                }
                return;
            }

            ESP_LOGI(tag, "[init]: Performing SPIFFS_check()");
            ret = esp_spiffs_check(cfg.partition_label);
            if (ret != ESP_OK)
            {
                __set_err(4);
                ESP_LOGE(tag, "SPIFFS_check() failed");
            }
            ESP_LOGI(tag, "[init]: Done");

            __meta_spiffs.set_state(driver_state_t::started);
            _initialized = true;
        }

        void __set_err(uint8_t code)
        {
            __meta_spiffs.inc_error(code);
        }

        void driver_deinit()
        {
            if (!_initialized)
            {
                return;
            }

            ESP_LOGI(tag, "[deinit]: Start\n");
            
            __meta_spiffs.set_state(driver_state_t::initialized, true);
            __meta_spiffs.set_state(driver_state_t::started, true);

            const auto& cfg = __cfg_spiffs;
            esp_err_t ret =  esp_vfs_spiffs_unregister(cfg.partition_label);
            if (ret != ESP_OK)
            {
                __set_err(5);
                ESP_LOGE(tag, "[deinit]: Unregister failed");
            }
        }

    };


    static u_driver_spiffs_t __driver_spiffs__instance = {};

}