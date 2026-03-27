#pragma once


#include "u_driver_wifi.h"

namespace __u_drivers
{

  // all method are public for direct call
    class u_driver_wifi_interface_t
    {
    public:
        using drv_t = u_driver_wifi_t;
        using mode_t = u_wifi_mode_t;
        using cfg_t = driver_wifi_cnf_t;
    private:
        drv_t& _driver;
    public:
        
        u_driver_wifi_interface_t(mode_t mode, cfg_t cfg, bool upd_cfg = true)
            :_driver(__driver_wifi_instance)
        {

            _driver.driver_init(mode, cfg);

            if (upd_cfg)
            {
                __cfg_wifi = cfg;
            }
        }

        // auto config for choosen mode from __cfg
        u_driver_wifi_interface_t(mode_t mode)
            :_driver(__driver_wifi_instance)
        {

            _driver.driver_init(mode, __cfg_wifi);
        }

        ~u_driver_wifi_interface_t()
        {
            _driver.driver_deinit();
        }

        // configurate current _active
        void ip_config(ufo::ip_t local_ip,
                ufo::ip_t gateway,
                ufo::ip_t subnet,
                ufo::ip_t dns1 = 0lu,
                ufo::ip_t dns2 = 0lu,
                ufo::ip_t dns3 = 0lu) 
        {
            auto act = _driver.get_active_mode();
            if (act == u_wifi_mode_t::ap)
            {
                _driver.get_native_ap()->ip_config(local_ip, gateway, subnet, dns1, dns2, dns3);
            }
            else if (act == u_wifi_mode_t::sta)
            {
                _driver.get_native_sta()->ip_config(local_ip, gateway, subnet, dns1, dns2, dns3);
            }
        }

        auto get_native_ap() 
        {
            return _driver.get_native_ap();
        }
        auto get_native_sta() 
        {
            return _driver.get_native_sta();
        }
    };
} // __u_drivers