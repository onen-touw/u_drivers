#pragma once

#include "impl/spiffs/u_driver_spiffs_interface.h"

namespace ufo
{
    using u_spiffs_t = __u_drivers::u_driver_spiffs_interface_t;
    using u_spiffs_cfg_t = __u_drivers::driver_spiffs_cnf_t;
    using u_spiffs_meta_t = __u_drivers::driver_meta_t;

    auto get_spiffs_driver_cfg() {
        u_spiffs_cfg_t cfg = __u_drivers::__cfg_spiffs;
        return cfg;
    }

    auto get_spiffs_driver_meta() {
        auto meta = __u_drivers::__meta_spiffs.get_meta();
        return meta;
    }

} // namespace ufo
