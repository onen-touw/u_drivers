#pragma once

#include "u_driver_spiffs.h"

namespace __u_drivers
{
    class u_driver_spiffs_interface_t
    {
    private:
        using drv_t = u_driver_spiffs_t;

        drv_t& _driver;
        bool _ifinit = false;
    
    public:
        struct fs_info_t
        {
            size_t total = 0, used = 0;
        };

    public:

        u_driver_spiffs_interface_t(const driver_spiffs_cnf_t& cfg) 
            : _driver(__driver_spiffs__instance) 
        {
            __info_spiffs.set_cfg(cfg);
            _driver.driver_init();
        }

        // WARNING: destructor do not deinit spiffs driver!!!
        u_driver_spiffs_interface_t() 
            : _driver(__driver_spiffs__instance) 
        {

            // get parameters from config. This parameters can be setted 
            // directly from code (bad way) or configurated via console and files
            // if driver is initialized when parameters are updated requare driver restart (or esp reboot if it saved in cfg-file)

            auto e = _driver.driver_init();
            if (e != ESP_OK)
            {
                return;
            }
            _ifinit = true;
            __info_spiffs.inc_user();
        }
            ~u_driver_spiffs_interface_t() 
        {
            if (!_ifinit)
            {
                return;
            }
            
            __info_spiffs.dec_user();
        }
        
        fs_info_t get_fs_info() const 
        {
            fs_info_t info = {};
            
            esp_err_t ret = esp_spiffs_info(_driver.get_root(), &info.total, &info.used);
            if (ret != ESP_OK)
            {
                // printf("Failed to get SPIFFS partition\n");
            }
            return info;
        }

        bool check_fs() const
        {
            esp_err_t ret = esp_spiffs_check(_driver.get_root());
            if (ret != ESP_OK)
            {
                return false;
            }
            return true;
        }

        bool mounted() const
        {
            return esp_spiffs_mounted(_driver.get_root());
        }
    

    };

}