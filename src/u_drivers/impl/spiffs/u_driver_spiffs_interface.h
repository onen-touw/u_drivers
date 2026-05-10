#pragma once

#include "u_driver_spiffs.h"

namespace __u_drivers
{
    class u_driver_spiffs_interface_t
    {
    private:
        using drv_t = u_driver_spiffs_t;

        drv_t& _driver;
    
    public:
        struct fs_info_t
        {
            size_t total = 0, used = 0;
        };

    public:
        // WARNING: destructor do not deinit spiffs driver!!!
        u_driver_spiffs_interface_t() 
            : _driver(__driver_spiffs__instance) 
        {

            // get parameters from config. This parameters can be setted 
            // directly from code (bad way) or configurated via console and files
            // if driver is initialized when parameters are updated requare driver restart (or esp reboot if it saved in cfg-file)

            const auto& cfg = __cfg_spiffs;
            _driver.driver_init(cfg);
            __meta_spiffs.inc_user();
        }
    
        fs_info_t get_fs_info() const 
        {
            fs_info_t info = {};
            
            const auto& cfg = __cfg_spiffs;

            esp_err_t ret = esp_spiffs_info(cfg.partition_label, &info.total, &info.used);
            if (ret != ESP_OK)
            {
                // printf("Failed to get SPIFFS partition\n");
            }
            return info;
        }

        bool check_fs() const
        {
            const auto& cfg = __cfg_spiffs;
            esp_err_t ret = esp_spiffs_check(cfg.partition_label);
            if (ret != ESP_OK)
            {
                return false;
            }
            return true;
        }

        bool mounted() const
        {
            return esp_spiffs_mounted(__cfg_spiffs.partition_label);
        }
    
        ~u_driver_spiffs_interface_t() 
        {
            __meta_spiffs.dec_user();
        }
    };

}