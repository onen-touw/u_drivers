#pragma once

#include "../u_driver_cfg.h"

namespace __u_drivers
{

    enum class u_wifi_mode_t
    {
        none,
        sta,
        ap,
    };

    struct cfg_wfm_t
    {
        const char* ss = "u_esp32";
		const char* ps = "12345678";
    };
    
    struct driver_wifi_cnf_t
    {
        cfg_wfm_t app = {};
        cfg_wfm_t sta = {};
        u_wifi_mode_t _auto_run = u_wifi_mode_t::none;

                //         int channel = 1, 
                // int ssid_hidden = 0, 
                // int max_connection = 4,
                // bool ftm_responder = false, 
    };

    // for driver state tracing 
    static driver_info_t __meta_wifi = {};

    // for auto config from .ini files and driver tracing
    static driver_wifi_cnf_t __cfg_wifi = {};

} // namespace __u_drivers
