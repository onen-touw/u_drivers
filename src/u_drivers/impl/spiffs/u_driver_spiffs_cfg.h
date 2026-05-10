#pragma once

#include <stdint.h>
#include "../u_driver_cfg.h"
#include "esp_spiffs.h"

namespace __u_drivers
{
    using driver_spiffs_cnf_t = esp_vfs_spiffs_conf_t;
    
    static driver_meta_registry __meta_spiffs = {};
    static driver_spiffs_cnf_t __cfg_spiffs = {"/spiffs", nullptr, 5, false };
}