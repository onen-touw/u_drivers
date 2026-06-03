#pragma once

#include "u_driver_spiffs_cfg.h"
#include "esp_log.h"

namespace __u_drivers
{
    class u_driver_spiffs_t
    {
    public: 
        static constexpr const char* tag = "spfs_drv";

    private:
        bool _initialized = false;
        const char* _root = nullptr;

    public:

        u_driver_spiffs_t() 
        {
            ESP_LOGI(tag, "[construct]");
        }
        ~u_driver_spiffs_t() 
        {
            ESP_LOGI(tag, "[destruct]");
           driver_deinit();
        }

        u_driver_spiffs_t(const u_driver_spiffs_t&) = delete;
        u_driver_spiffs_t(u_driver_spiffs_t&&) = delete;

        const char* get_root() const 
        {
            return _root;
        }

        esp_err_t driver_init()
        {
            if (_initialized)
            {
                // in this case its ok
                return ESP_OK;
            }

            ESP_LOGI(tag, "[init]: Start");

            auto cfg = __info_spiffs.get_cfg();

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
                return ret;
            }

            ESP_LOGI(tag, "[init]: Performing SPIFFS_check()");
            ret = esp_spiffs_check(cfg.partition_label);
            if (ret != ESP_OK)
            {
                __set_err(4);
                ESP_LOGE(tag, "SPIFFS_check() failed");
                return ret;
            }
            ESP_LOGI(tag, "[init]: Done");

            __info_spiffs.set_state(driver_state_t::initialized);
            __info_spiffs.set_state(driver_state_t::started);

            _initialized = true;
            _root = cfg.partition_label;
            return ret;
        }

        void __set_err(uint8_t code)
        {
            __info_spiffs.inc_error(code);
        }

        void driver_deinit()
        {
            if (!_initialized)
            {
                return;
            }

            ESP_LOGI(tag, "[deinit]");
            
            __info_spiffs.set_state(driver_state_t::initialized, true);
            __info_spiffs.set_state(driver_state_t::started, true);

            auto cfg = __info_spiffs.get_cfg();
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