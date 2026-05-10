#pragma once

#include "u_driver_wifi_cfg.h"

#include "wifi_impl/wfap.h"
#include "wifi_impl/wfsta.h"
#include <bits/unique_ptr.h>

#include "nvs_flash.h"

namespace __u_drivers
{

    // wrapper about ufo implementation
    // Boris if you want you can refactor my implementation yourself and suck my dick!!! HAHA
    class u_driver_wifi_t
    {
    public:
        using ap_t = ufo::wf_details::wfap_t;
        using sta_t = ufo::wf_details::wfsta_t;
    private:
        std::unique_ptr<ap_t>    _ap;
        std::unique_ptr<sta_t>   _sta;
        u_wifi_mode_t _active = u_wifi_mode_t::none;
        bool _nvs = false;

    public:
        u_driver_wifi_t()
        {
            _ap = std::make_unique<ap_t>();
            _sta = std::make_unique<sta_t>();
        }
        ~u_driver_wifi_t(){}

        void driver_init(u_wifi_mode_t mode, driver_wifi_cnf_t cfg) 
        {
            nvs_init();
            swap_wf_mode(mode, cfg);

        }

        void driver_deinit() 
        {
            swap_wf_mode(u_wifi_mode_t::none, __cfg_wifi);
        }

        u_wifi_mode_t get_active_mode() const { return _active; }

        // configurate current _active
        void ip_config(ufo::ip_t local_ip,
                ufo::ip_t gateway,
                ufo::ip_t subnet,
                ufo::ip_t dns1 = 0lu,
                ufo::ip_t dns2 = 0lu,
                ufo::ip_t dns3 = 0lu) 
        {
            if (_active == u_wifi_mode_t::ap)
            {
                _ap->ip_config(local_ip, gateway, subnet, dns1, dns2, dns3);
            }
            else if (_active == u_wifi_mode_t::sta)
            {
                _sta->ip_config(local_ip, gateway, subnet, dns1, dns2, dns3);
            }
        }

        void swap_wf_mode(u_wifi_mode_t mode, const driver_wifi_cnf_t& cfg)
        {
            // if sta or app running disable it           
            if (_active == u_wifi_mode_t::ap)
            {
                _ap->disable();
            }
            else if (_active == u_wifi_mode_t::sta)
            {
                _sta->disable();
            }

            // if run choosen. if None just return       
            if (mode == u_wifi_mode_t::ap)
            {
                _ap->enable();
                _ap->create(cfg.app.ss, cfg.app.ps);
            }
            else if (mode == u_wifi_mode_t::sta)
            {
                _sta->enable();
                _sta->connect(cfg.sta.ss, cfg.sta.ps);
            }
            _active = mode;
        }

        auto get_native_ap()
        {
            return _ap.get();
        }
        auto get_native_sta()
        {
            return _sta.get();
        }

    private:
        void nvs_init() {
            if (!_nvs)
            {
                esp_err_t ret = nvs_flash_init();
                if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
                {
                    ESP_ERROR_CHECK(nvs_flash_erase());
                    ret = nvs_flash_init();
                    if (ret != ESP_OK)
                    {
                        //todo meta
                    }
                }
                _nvs = true;
            }
        }
    };

    static u_driver_wifi_t __driver_wifi_instance;
    
} // __u_drivers